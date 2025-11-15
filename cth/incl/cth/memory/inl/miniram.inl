#pragma once
#include "cth/io/log.hpp"

#include <algorithm>
#include <numeric>

namespace cth::mem {

// Float conversion helper functions
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::highest_bit(size_type number) -> size_type {
    constexpr size_type bits = sizeof(size_type) * 8;
    return bits - 1 - std::countl_zero(number);
}

template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::ceil_to_float(size_type size) -> size_type {
    size_type exp = 0;
    size_type mantissa = 0;

    if(size < MANTISSA_VALUE) { mantissa = size; } else {
        auto const mantissaStartBit = highest_bit(size) - MANTISSA_BITS;
        exp = mantissaStartBit + 1;
        mantissa = (size >> mantissaStartBit) & MANTISSA_MASK;

        auto const lowBitsMask = (size_type{1} << mantissaStartBit) - 1;

        // Round up!
        if((size & lowBitsMask) != 0)
            mantissa++;
    }

    return (exp << MANTISSA_BITS) + mantissa;
}

template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::floor_to_float(size_type size) -> size_type {
    if(size < MANTISSA_VALUE)
        return size;

    auto const mantissaBegin = highest_bit(size) - MANTISSA_BITS;
    auto const exp = mantissaBegin + 1;
    auto const mantissa = (size >> mantissaBegin) & MANTISSA_MASK;

    return (exp << MANTISSA_BITS) | mantissa;
}

template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::to_uint(size_type float_value) -> size_type {
    if(float_value < MANTISSA_VALUE)
        return float_value;

    auto const exp = float_value >> MANTISSA_BITS;
    auto const mantissa = float_value & MANTISSA_MASK;
    return (mantissa | MANTISSA_VALUE) << (exp - 1);
}

// Constructor
template<uint SizeType, uint IndexType>
constexpr basic_miniram<SizeType, IndexType>::basic_miniram(size_type capacity, size_t initial_alloc_capacity) :
    _capacity(capacity),
    _maxAllocs(initial_alloc_capacity) { clear(); }

// Clear
template<uint SizeType, uint IndexType>
constexpr void basic_miniram<SizeType, IndexType>::clear() {
    defragmentReset();

    _freeStackPtr = 0;

    auto const allocCapacity = std::exchange(_maxAllocs, size_t{0});
    reserve_allocations(allocCapacity);

    insertNode(capacity(), 0);
}

// Insert Node
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::insertNode(size_type size, size_type data_offset) -> index_type {
    auto const nodeIndex = newNode(size);

    auto const binIndex = floor_to_float(size);

    auto const topBinIndex = binIndex >> TOP_BINS_INDEX_SHIFT;
    auto const leafBinIndex = binIndex & LEAF_BINS_INDEX_MASK;

    if(_binIndices[binIndex] == INVALID_INDEX) {
        _usedBinsTop |= top_bin_mask_t{1} << topBinIndex;
        _usedBins[topBinIndex] |= leaf_bin_mask_t{1} << leafBinIndex;
    }

    index_type const listPos = _binIndices[binIndex];

    _nodes[nodeIndex] = {
        .dataOffset = data_offset,
        .dataSize = size,
        .binListNext = listPos
    };

    if(listPos != INVALID_INDEX)
        _nodes[listPos].binListPrev = nodeIndex;

    _binIndices[binIndex] = nodeIndex;

    return nodeIndex;
}

// Remove Node
template<uint SizeType, uint IndexType>
constexpr void basic_miniram<SizeType, IndexType>::removeNode(index_type node_index) {
    auto const& node = _nodes[node_index];

    if(node.binListPrev != INVALID_INDEX) {
        _nodes[node.binListPrev].binListNext = node.binListNext;

        if(node.binListNext != INVALID_INDEX)
            _nodes[node.binListNext].binListPrev = node.binListPrev;
    } else {
        auto const binIndex = floor_to_float(node.dataSize);

        auto const topBinIndex = binIndex >> TOP_BINS_INDEX_SHIFT;
        auto const leafBinIndex = binIndex & LEAF_BINS_INDEX_MASK;

        _binIndices[binIndex] = node.binListNext;

        if(node.binListNext != INVALID_INDEX)
            _nodes[node.binListNext].binListPrev = INVALID_INDEX;

        if(_binIndices[binIndex] == INVALID_INDEX) {
            _usedBins[topBinIndex] &= ~(leaf_bin_mask_t{1} << leafBinIndex);

            if(_usedBins[topBinIndex] == 0)
                _usedBinsTop &= ~(top_bin_mask_t{1} << topBinIndex);
        }
    }

    freeNode(node_index);
}

// New Node
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::newNode(size_type size) -> index_type {
    _freeStorage += size;
    return popFreeNode();
}

// Free Node
template<uint SizeType, uint IndexType>
constexpr void basic_miniram<SizeType, IndexType>::freeNode(index_type node_index) {
    CTH_CRITICAL(node_index >= nodes(), "invalid node index") {}

    _freeStorage -= _nodes[node_index].dataSize;

    pushFreeNode() = node_index;
}

// Defragment
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::defragment(size_type new_size) -> defrag_type {
    defrag_type report{};

    std::vector<index_type> usedNodes{};
    usedNodes.reserve(allocated());

    _freeStackPtr = 0;
    for(size_t i = 0; i < nodes(); ++i) {
        if(_nodes[i].used)
            usedNodes.push_back(static_cast<index_type>(i));
        else
            _freeNodes[_freeStackPtr++] = static_cast<index_type>(i);
    }

    size_type compactedOffset = 0;
    index_type lastUsedNode = INVALID_INDEX;

    if(!usedNodes.empty()) {
        std::ranges::sort(
            usedNodes,
            [&](index_type a, index_type b) { return _nodes[a].dataOffset < _nodes[b].dataOffset; }
        );

        compactedOffset = compactUsedNodes(usedNodes, report);
        lastUsedNode = usedNodes.back();
    }

    // Update capacity before reconstructing free space
    _capacity = new_size;

    // Clear bin metadata and rebuild free space
    defragmentReset();
    if(_capacity > compactedOffset)
        reconstructFreeSpace(compactedOffset, lastUsedNode);

    return report;
}

// Allocate
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::allocate(size_type size) -> alloc_type {
    if(size > remaining())
        return {.offset = NO_SPACE, .id = INVALID_INDEX};

    if(remaining_allocs() == 0)
        reserve_allocations();

    auto const minBinId = ceil_to_float(size);

    auto const minTopBinId = minBinId >> TOP_BINS_INDEX_SHIFT;
    auto const minLeafBinId = minBinId & LEAF_BINS_INDEX_MASK;

    size_t topBinIndex = minTopBinId;
    size_t leafBinIndex = static_cast<size_t>(-1);

    if(_usedBinsTop & (top_bin_mask_t{1} << topBinIndex))
        leafBinIndex = findLowestBitAfter(_usedBins[topBinIndex], minLeafBinId);

    if(leafBinIndex == static_cast<size_t>(-1)) {
        topBinIndex = findLowestBitAfter(_usedBinsTop, minTopBinId + 1);

        if(topBinIndex == static_cast<size_t>(-1))
            return {.offset = NO_SPACE, .id = INVALID_INDEX};

        leafBinIndex = std::countr_zero(_usedBins[topBinIndex]);
    }

    auto const binIndex = (topBinIndex << TOP_BINS_INDEX_SHIFT) | leafBinIndex;

    auto const nodeId = _binIndices[binIndex];

    sliceTopBinNode(size, binIndex, topBinIndex, leafBinIndex);

    return {.offset = _nodes[nodeId].dataOffset, .id = nodeId};
}

// To Disjunct Copies
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::to_disjunct_copies(
    std::vector<memmove_type> const& moves
)
    -> std::vector<memmove_type> {
    std::vector<memmove_type> disjunctMoves{};
    disjunctMoves.reserve(moves.size() * 2);

    for(auto const& move : moves) {
        CTH_CRITICAL(move.dstOffset > move.srcOffset, "This function does not support 'move-right' operations.") {}

        auto const chunkSize = move.srcOffset - move.dstOffset;
        bool const overlap = move.size > chunkSize;

        if(!overlap) {
            disjunctMoves.push_back(move);
            continue;
        }

        auto remainingSize = move.size;
        auto currentSrc = move.srcOffset;
        auto currentDst = move.dstOffset;

        auto const numChunks = (remainingSize + chunkSize - 1) / chunkSize;
        disjunctMoves.reserve(disjunctMoves.size() + numChunks);

        while(remainingSize > chunkSize) {
            disjunctMoves.emplace_back(currentSrc, currentDst, chunkSize);
            remainingSize -= chunkSize;
            currentSrc += chunkSize;
            currentDst += chunkSize;
        }

        if(remainingSize > 0)
            disjunctMoves.emplace_back(currentSrc, currentDst, remainingSize);
    }
    return disjunctMoves;
}

// Reserve Nodes
template<uint SizeType, uint IndexType>
constexpr void basic_miniram<SizeType, IndexType>::reserve_nodes(size_t node_capacity) {
    if(node_capacity <= nodes())
        return;

    auto const oldCapacity = nodes();

    _nodes.resize(node_capacity);
    _freeNodes.resize(node_capacity);

    std::iota(_freeNodes.begin() + oldCapacity, _freeNodes.end(), static_cast<index_type>(oldCapacity));
}

// Slice Top Bin Node
template<uint SizeType, uint IndexType>
constexpr void basic_miniram<SizeType, IndexType>::sliceTopBinNode(
    size_type slice_size,
    size_t bin_id,
    size_t top_bin_id,
    size_t leaf_bin_id
) {
    auto const originalNodeId = _binIndices[bin_id];
    auto& node = _nodes[originalNodeId];

    auto const nodeTotalSize = node.dataSize;

    _freeStorage -= nodeTotalSize;

    node.dataSize = slice_size;
    node.used = true;

    _binIndices[bin_id] = node.binListNext;

    if(node.binListNext != INVALID_INDEX)
        _nodes[node.binListNext].binListPrev = INVALID_INDEX;

    if(_binIndices[bin_id] == INVALID_INDEX) {
        _usedBins[top_bin_id] &= ~(leaf_bin_mask_t{1} << leaf_bin_id);

        if(_usedBins[top_bin_id] == 0)
            _usedBinsTop &= ~(top_bin_mask_t{1} << top_bin_id);
    }

    auto const reminderSize = nodeTotalSize - slice_size;
    if(reminderSize > 0) {
        auto const newNodeIndex = insertNode(reminderSize, node.dataOffset + slice_size);
        auto& newNode = _nodes[newNodeIndex];

        if(node.neighborNext != INVALID_INDEX)
            _nodes[node.neighborNext].neighborPrev = newNodeIndex;

        newNode.neighborPrev = originalNodeId;
        newNode.neighborNext = node.neighborNext;

        node.neighborNext = newNodeIndex;
    }
}

// Find Lowest Bit After
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::findLowestBitAfter(top_bin_mask_t bit_mask, size_t start_id) -> size_t {
    auto const maskBefore = (top_bin_mask_t{1} << start_id) - 1;
    auto const maskAfter = ~maskBefore;
    auto const bitsAfter = bit_mask & maskAfter;

    if(bitsAfter == 0) return static_cast<size_t>(-1);

    return std::countr_zero(bitsAfter);
}

// Free
template<uint SizeType, uint IndexType>
constexpr void basic_miniram<SizeType, IndexType>::free(alloc_type allocation) {
    if(_nodes.empty())
        return;

    auto const nodeId = allocation.id;
    auto& node = _nodes[nodeId];

    CTH_CRITICAL(!node.used, "cannot free an already freed node") {}

    auto offset = node.dataOffset;
    auto size = node.dataSize;
    auto prevNeighbor = node.neighborPrev;
    auto nextNeighbor = node.neighborNext;

    // Merge with previous free neighbor
    if(prevNeighbor != INVALID_INDEX && !_nodes[prevNeighbor].used) {
        auto const& prevNode = _nodes[prevNeighbor];
        CTH_CRITICAL(prevNode.neighborNext != nodeId, "Neighbor chain corrupted: prev->next != current") {}

        offset = prevNode.dataOffset;
        size += prevNode.dataSize;
        prevNeighbor = prevNode.neighborPrev;

        removeNode(node.neighborPrev);
    }

    // Merge with next free neighbor
    if(nextNeighbor != INVALID_INDEX && !_nodes[nextNeighbor].used) {
        auto const& nextNode = _nodes[nextNeighbor];
        CTH_CRITICAL(nextNode.neighborPrev != nodeId, "Neighbor chain corrupted: next->prev != current") {}

        size += nextNode.dataSize;
        nextNeighbor = nextNode.neighborNext;

        removeNode(node.neighborNext);
    }

    // Return current node to free pool
    pushFreeNode() = nodeId;

    // Create merged free node
    auto const combinedNodeIndex = insertNode(size, offset);
    auto& combinedNode = _nodes[combinedNodeIndex];

    // Reconnect neighbor chain
    combinedNode.neighborPrev = prevNeighbor;
    combinedNode.neighborNext = nextNeighbor;

    if(nextNeighbor != INVALID_INDEX)
        _nodes[nextNeighbor].neighborPrev = combinedNodeIndex;

    if(prevNeighbor != INVALID_INDEX)
        _nodes[prevNeighbor].neighborNext = combinedNodeIndex;
}

// Size Of
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::size_of(alloc_type allocation) const -> size_type {
    if(allocation.id == INVALID_INDEX || _nodes.empty())
        return 0;

    return _nodes[allocation.id].dataSize;
}

// Max Alloc
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::max_alloc() const -> size_type {
    if(remaining() == 0) return 0;

    size_t const topBinIndex = highest_bit(_usedBinsTop);
    size_t const leafBinIndex = highest_bit(static_cast<size_type>(_usedBins[topBinIndex]));
    size_t const finalBinIndex = (topBinIndex << TOP_BINS_INDEX_SHIFT) | leafBinIndex;

    return to_uint(static_cast<size_type>(finalBinIndex));
}

// Defragment Reset
template<uint SizeType, uint IndexType>
constexpr void basic_miniram<SizeType, IndexType>::defragmentReset() {
    _usedBinsTop = 0;
    _usedBins.fill(0);
    _binIndices.fill(INVALID_INDEX);
    _freeStorage = 0;
}

// Pop Free Node
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::popFreeNode() -> index_type {
    CTH_CRITICAL(_freeStackPtr >= nodes(), "free stack already empty") {}

    return _freeNodes[_freeStackPtr++];
}

// Push Free Node
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::pushFreeNode() -> index_type& {
    CTH_CRITICAL(_freeStackPtr == 0, "free stack already full") {}

    return _freeNodes[--_freeStackPtr];
}

// Compact Used Nodes
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::compactUsedNodes(
    std::vector<index_type> const& used_nodes,
    defrag_type& report
) -> size_type {
    report.updatedAllocs.reserve(used_nodes.size());
    report.moves.reserve(used_nodes.size());

    size_type compactedOffset = 0;
    index_type prevNodeId = INVALID_INDEX;
    size_type prevNodeOldOffset = 0;

    for(auto const currentNodeID : used_nodes) {
        auto& node = _nodes[currentNodeID];
        auto const oldOffset = node.dataOffset;
        auto const nodeSize = node.dataSize;

        report.updatedAllocs.emplace_back(compactedOffset, currentNodeID);

        if(oldOffset != compactedOffset) {
            bool const contiguous = prevNodeId != INVALID_INDEX
                && oldOffset == prevNodeOldOffset + _nodes[prevNodeId].dataSize;

            if(contiguous && !report.moves.empty())
                report.moves.back().size += nodeSize;
            else
                report.moves.emplace_back(oldOffset, compactedOffset, nodeSize);
        }

        node.dataOffset = compactedOffset;
        node.neighborPrev = prevNodeId;

        if(prevNodeId != INVALID_INDEX)
            _nodes[prevNodeId].neighborNext = currentNodeID;

        compactedOffset += nodeSize;
        prevNodeId = currentNodeID;
        prevNodeOldOffset = oldOffset;
    }

    _nodes[prevNodeId].neighborNext = INVALID_INDEX;

    report.moves.shrink_to_fit();

    return compactedOffset;
}

// Reconstruct Free Space
template<uint SizeType, uint IndexType>
constexpr void basic_miniram<SizeType, IndexType>::reconstructFreeSpace(
    size_type compacted_offset,
    index_type last_used_node
) {
    size_type const totalFreeSize = capacity() - compacted_offset;
    if(totalFreeSize == 0)
        return;

    // Split free space into main block (quantized) and sliver (remainder)
    auto const mainBlockBin = floor_to_float(totalFreeSize);
    auto const mainBlockSize = to_uint(mainBlockBin);
    auto const sliverSize = totalFreeSize - mainBlockSize;

    index_type previousFreeNode = last_used_node;

    // Insert main free block
    if(mainBlockSize > 0) {
        auto const mainNode = insertNode(mainBlockSize, compacted_offset);
        auto& mainNodeRef = _nodes[mainNode];

        mainNodeRef.used = false;
        mainNodeRef.neighborPrev = previousFreeNode;

        if(previousFreeNode != INVALID_INDEX)
            _nodes[previousFreeNode].neighborNext = mainNode;

        previousFreeNode = mainNode;
    }

    // Insert sliver free block (if any remainder exists)
    if(sliverSize > 0) {
        auto const sliverNode = insertNode(sliverSize, compacted_offset + mainBlockSize);
        auto& sliverNodeRef = _nodes[sliverNode];

        sliverNodeRef.used = false;
        sliverNodeRef.neighborPrev = previousFreeNode;

        if(previousFreeNode != INVALID_INDEX)
            _nodes[previousFreeNode].neighborNext = sliverNode;
    }
}

// Free Regions
template<uint SizeType, uint IndexType>
constexpr auto basic_miniram<SizeType, IndexType>::free_regions() const -> std::array<regions_type, NUM_LEAF_BINS> {
    std::array<regions_type, NUM_LEAF_BINS> regions{};

    for(size_t i = 0; i < NUM_LEAF_BINS; ++i) {
        regions[i].size = to_uint(static_cast<size_type>(i));
        regions[i].count = 0;

        auto nodeIndex = _binIndices[i];
        while(nodeIndex != INVALID_INDEX) {
            regions[i].count++;
            nodeIndex = _nodes[nodeIndex].binListNext;
        }
    }

    return regions;
}

}
