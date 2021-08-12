# Versus

*Versus* is a bare minimum yet powerful CPython Extension module, intended to ensure both perfect accuracy and reasonable performance while solving the [Longest Common Subsequence](https://en.wikipedia.org/wiki/Longest_common_subsequence_problem) problem. 

The provided implementation is particularly well suited for the comparison of incremental changes on version control systems, although you can tackle a number of [bioinformatics](https://en.wikipedia.org/wiki/Bioinformatics) and [computational linguistics](https://en.wikipedia.org/wiki/Computational_linguistics) use cases with it too.

## The algorithm(s)

With [An O(ND) Difference Algorithm and Its Variations](http://www.xmailserver.org/diff2.pdf), Eugene Myers laid the foundations for what would later become the building blocks of pretty much every *diffing tool* coders work with today. Since its publication in 1986, the *linear space* refinement algorithm (pp. 10-12) have managed to find their way into the most recent versions of [Git Diff](https://github.com/git/git/blob/master/xdiff/xdiffi.c), [GNU Diff](https://github.com/freebsd/freebsd-src/blob/master/contrib/diff/src/analyze.c) and Google's [Diff Match Patch](https://opensource.google/projects/diff-match-patch). 

There is a catch, though; in order to overcome the [once impractical] space complexity of the standard [Dynamic Programming](https://en.wikipedia.org/wiki/Dynamic_programming) approach found in the original *greedy* design (p. 6, same paper), the aforementioned *divide and conquer* alternative needs to compromise its alignment quality under certain [common circumstances](https://blog.jcoglan.com/2017/09/19/the-patience-diff-algorithm/). 

Visual annoyances aside, the resulting *sliding* side effect can be a real problem for *diff patching* or any other sensitive application (like, for example, the kind of [project]() Versus was designed to be part of in the first place). 

Up to a point, the issue can still be mitigated with strategies like Bram Cohen's [Patience](https://stackoverflow.com/questions/4045017/what-is-git-diff-patience-for) *preprocessor*. Unfortunately, the underlying trade-off, inherent to the D&C variation, is not going anywhere; it's difficult to enjoy a fully deterministic LCS/SES output and overcome the traditional DP's efficiency barrier at the same time.

Yet another option would be to combine Myers's first bid with a couple of well-known DPA optimizations, such as: 
* [Ukkonen's k-band](https://www.sciencedirect.com/science/article/pii/S0019995885800462/pdf), for a dynamically bounded search space (as Robert Elder explains very well [here](https://blog.robertelder.org/diff-algorithm/)).
* [Hunt's k-candidates](https://www.cs.dartmouth.edu/~doug/diff.pdf), for a better vector storage management (as Tony Garnock-Jones does [here](https://gist.github.com/tonyg/2361e3bfe4e92a1fc6f7)).

This, in addition to some [delta encoding](https://en.wikipedia.org/wiki/Delta_encoding) to further reduce the space complexity, is exactly what *Versus* does. 

>**NOTE** that, even when you could get even better results with [edlib](https://github.com/Martinsos/edlib) or a similar bit-vector based [implementation](http://www.gersteinlab.org/courses/452/09-spring/pdf/Myers.pdf) (a *Myers's 2.0* of sorts, still DP & about 10x faster; per core), the alphabet size would be limited to 128 unique symbols (far below the thousands of unique lines your code can have) and reusability would be severily hindered for any full-fledged library intended to be built on top of it (*Versus*, in this regard, is dead simple and portable).

## Performance

Below you can find how *Versus* behaves at runtime when comparing up to one million objects (*lines of code*, in this case). Even though the data points have been rounded for readability purposes, the measurements are pretty close to what you can expect for a modern mid-range desktop machine (like, for example, the [Geekbench 5] [850/SC](https://browser.geekbench.com/processors/intel-core-i7-4870hq) I used).

A *lag line* is drawn in order to let you know when you might start to experience some delay. This will not ever happen while working on an incremental basis (this is what *Versus* has been specifically created for), but you will need to wait for about 100 milliseconds if your code reaches the [2000 LOC](https://softwareengineering.stackexchange.com/questions/176999/at-what-point-range-is-a-code-file-too-big) barrier without a single match on the opposite side.
![Performance](https://user-images.githubusercontent.com/3150023/129176398-0595db5a-ad28-4fe9-83dc-9290857dbda5.png)

>**NOTE** that even though DP's [worst case scenario] quadratic space complexity is usually not critical when working with VCS, in extreme circumstances (like the *half-a-million-of-unmatchable-lines-of-code* one), *Versus* could take up to 64 GB of virtual memory. Beyond that limit (as you can read in the *Error handling* section [below](#error-handling)), the LCS/SES is just an estimate.

## Installation

First, let's make sure the platform is ready; here are the components your system may require:

OS | Environment | Compiler 
------------- | ------------ | -------------
Windows 7SP1&/8.1/10 | Windows 10 SDK (10.0.x.x) | MSVC v14x (VS 201x C++ x64/x86)
MacOS 11.x | `sudo xcode-select --install` | `sudo xcode-select --install`
Fedora 3x.x | `sudo dnf install python3-devel` | *Preinstalled*
Ubuntu 2x.x | `sudo apt-get install python3-dev` | `sudo apt-get install gcc`

>**NOTE** that the required Windows components can be found in the *Desktop development with C++ Workload* included with the *Microsoft Build Tools for Visual Studio 201x* [installer](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools) (or, from any [VS edition](https://visualstudio.microsoft.com/vs/older-downloads/), by selecting *Python native development tools* found in the *Python Development Workload*). Windows 7SP1 users will need to have [Microsoft .NET Framework](https://dotnet.microsoft.com/download/dotnet-framework) 4.5 or greater installed (in addition to [this patch](https://stackoverflow.com/questions/58548069/installing-python-3-8-on-windows-7-32bit-with-sp1), for [Python 3.7/3.8](https://www.python.org/downloads/windows/) support) before starting VS.

Now we can compile `versus.c` into the binary module that our **Python 3.6+** scripts will be able to load later:

```
python3 setup.py build
```

In order to install the module, just move the `/build/foo/versus.os.so|pyc` binary just created (tip: make it version-agnostic by renaming the file to `versus.so|pyd`) into your registered `site-packages` path of choice; as per:

```
python3 -m site
```

We are done; let's test the code with something simple:

```
import versus
print(versus.lcs(["t", "o"], ["a", "t"]))
print(versus.ses(["t", "o"], ["a", "t"]))
```

## Use

In its current form, Versus is designed to be compact and reusable, without any kind of extended functionality built on top of it. As a result, none of the two LCS/SES outputs are human-readable at first glance.

Having said that, you can learn everything you need from the following example:

![Use](https://user-images.githubusercontent.com/3150023/129176442-edd8bc70-b181-4d84-9c6b-d3eae8518d05.png)

>**NOTE** that the *Shortest Edit Script* contains only two types of commands; `-1` *deletions* from file A and `1` *insertions* into file B. In order to support *substitutions*, while keeping the algorithm's expected behaviour intact, *Versus* renames the operation type flag `-1` as `0` whenever a deletion is known to be followed by an overlapping insertion, so downstream code can easily tell (or ignore) the difference. 

## Error handling

*Versus* is able to compare just about any kind of built-in Python object against each other (nested structures included), provided that they are fed into the module as a couple of lists. Otherwise, the program will exit with a `bad input type` exception. 

Another `bad input size` exception will be thrown if either of the two input sequences exceeds `4294967295` items in size, or is empty.

Finally, in order to be able to dynamically allocate enough space for the main algorithm, a preliminary memory check is carried out. If your addressable VM turns out to be insufficient, one last `lack of memory` exception will follow.

As for warnings, a `too many edges` message could be written to `stdout` just before the output is printed. This is a fallback mechanism for those fringe cases where the number of matching edges reach the `UINT_MAX` limit (yep, as the inputs). When that happens, *Versus* stops looking for the LCS/SES, and the best available alternative is backtraced instead.

>**NOTE** that the *good enough* contingency logic from above is very similar to the [usually enabled by default] *non-minimal* heuristic speed-up you can find in most Myers's D&C implementatiions.
