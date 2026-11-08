# Agent guide

How to write documentation in this repository.
Read it before you change a file. Match the existing code.

## Documentation style

Documentation uses doxygen comments and follows ASD-STE100 (Simplified Technical English), adapted to the short style of this code base.

### Form

- Use `/** ... */` blocks with one tag per line.
- The first line of a block is the brief: what it does, in one line, without a tag. Doxygen reads it with `JAVADOC_AUTOBRIEF`.
- Start after the tag with a lower case letter.
- Do not put a period at the end of a line.
- Write short fragments, not full sentences. Start them with a verb in the present tense: `creates the fence`, `posts the work`.
- Fragments win over STE100: where STE100 asks for a full sentence or an article, write the shorter fragment.
- Put every piece of code in backticks: our names, std names, values, parameter names, expressions:
  `` `start()` ``, `` `scheduler::owns_thread()` ``, `` `std::nullopt` ``, `` `workers` ``, `` `x < y` ``.
  Doxygen links our names inside backticks by itself (autolink), CLion shows backticks as code.
- `@ref` only where autolink does not resolve a name; a member variable is qualified instead: `` `scheduler::_workers` ``.
- Do not document what the signature already tells. A getter without conditions needs no comment.

### Tags

| Tag | Use |
|---|---|
| `@details` | more information after the brief. `calls:` followed by a list of the functions it calls |
| `@param` | name, then what it is for: `@param work to run` |
| `@tparam` | template parameter, same form as `@param` |
| `@return` | what the result is |
| `@pre` | condition that must be true before the call: ``@pre `active()` `` |
| `@post` | condition that is true after the call: ``@post `signaled()` `` |
| `@throws` | exception type and the cause: `@throws cth::except::default_exception reason: file not found`; the exception type stays raw, doxygen links it |
| `@note` | side effect the caller must know: `@note resets the fence` |
| `@attention` | danger that is not a precondition: undefined behavior, not thread safe |

- Write preconditions as `@pre`, not as `@attention requires ...`.
- For an exception of this library, write the cause after `reason:`.

### Math like notation

Use symbols when they are shorter and clearer than words:

- implication: `` `nullptr` -> no job of a context ``
- conditions and ranges: `` @pre `0 < workers < MAX_INT` ``
- special values: `` (`INFINITE_WAIT` => no timeout) ``
- the expression goes in backticks, the arrow and the words stay outside

### STE100 rules that apply

1. Use one word for one meaning. Do not use synonyms. The terms in the table below are fixed.
2. Write descriptions with 25 words or fewer and instructions with 20 words or fewer.
3. Write one topic in one fragment. Split two actions into two lines.
4. Use the active voice and the present tense.
5. Write instructions to the caller in the imperative: ``call `start()` first``.
6. Do not use phrasal verbs (`set up`, `carry out`). Use one verb (`create`, `do`).
7. Do not use `-ing` forms as nouns or adjectives.
8. Use `it`, `this` and `they` only when the reference is clear.
9. Write the most important condition first, then the consequence.

| Term | Meaning |
|---|---|
| construct | the C++ object comes into existence |
| create | the OS or asio object behind a C++ object comes into existence |
| destroy | that object stops to exist |
| release | give the ownership to the caller without destroying |
| handle | the raw OS handle, for example `native_handle_t` |
| task | a coroutine of this library, its return type derives from `task_base` |
| caller | the coroutine that awaits a task or an awaiter |
| context | where coroutines run and wait: a `scheduler` or a `strand` |
| payload | the user data that a task gets from its caller |
| post | give work to a context, the work runs later |
| job | work that a context runs |
| worker | a thread of a scheduler |
| coroutine | the coroutine frame behind a `std::coroutine_handle` (handle stays the OS handle) |
| this_coro coroutine | a coroutine whose promise derives `this_coro_base` |
| foreign awaitable | an awaitable without this_coro support |
| calling thread | the thread that calls the function, a fixed exception to rule 7 |

### Example

```cpp
/**
 * spawns the task and blocks until it ends
 * @tparam Pyld payload of the task, constructed from the arguments
 * @param task to run on this context
 * @param args to construct the payload with
 * @return result of the task
 * @attention blocks the calling thread
 */
template<payload Pyld = this_coro::default_payload, awaitable Awaitable, class... Args>
auto sync_wait(Awaitable task, Args&&... args) const -> awaited_t<Awaitable>;
```

### When you change code

- Check every comment against the code. Old comments are not always up to date.
- Report bugs that you find. Do not fix a behavior change silently.

### Repository text

- Do not use em dashes or en dashes in comments, docs, commit messages or markdown.
