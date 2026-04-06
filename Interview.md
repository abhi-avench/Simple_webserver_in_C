- What is static analysis?
- Why use static analysis in embedded C?
- Difference between static analysis and dynamic testing?
- Difference between compiler warnings and lint/static analysis?
- What is Cppcheck?
- What is PC-lint Plus?
- What is MISRA?
- How do you run Cppcheck on a CMake project?
- Why use compile_commands.json?
- What kinds of defects can static analysis catch?
- What are false positives and false negatives?
- What is a suppression?
- What is a MISRA deviation?
- Can static analysis prove code is bug-free?
- How do you integrate lint tools into CI?
- How do you deal with warnings from legacy code?
- Why are ignored return values important?
- Why is signed/unsigned conversion dangerous?
- What is dead code?
- What is unreachable code?
- Why is null-pointer analysis useful?
- Why is bounds checking important?
- How do coding standards help safety-critical software?

# Best answers interviewers usually like to hear

A strong answer often sounds like this:

“We use static analysis tools such as Cppcheck or PC-lint Plus to detect defects, undefined behavior, and coding-standard violations early, before runtime testing. In embedded projects, we typically integrate them with the build system, use the compile database for accuracy, review findings, suppress only justified cases, and track MISRA deviations formally.”
This aligns with the documented positioning of both tools around defects and compliance.

# Best 15 interview questions to prepare first

# If you want the highest-value shortlist, prepare these:

- What is static analysis?
- Why use static analysis in embedded C?
- Difference between static analysis and testing?
- Difference between compiler warnings and lint tools?
- What is Cppcheck?
- What is PC-lint Plus?
- What types of bugs do they find?
- What is MISRA compliance?
- How does Cppcheck MISRA checking work?
- What is compile_commands.json?
- What are false positives and false negatives?
- What is a suppression/deviation?
- How would you integrate static analysis into CI?
- How do you reduce noise in reports?
- Can static analysis replace code review or testing?

If you want, I can turn this into a 1-page interview cheat sheet with model answers for Cppcheck, PC-lint, and MISRA.