# CAR!

# Contributing

## Coding guidelines

The project follows a set of coding guidelines to keep the code consistent between the files. The guidelines have been adopted from the Mono project and can be found [here](https://github.com/mono/website/blob/gh-pages/community/contributing/coding-guidelines.md) ([alternative link](https://www.mono-project.com/community/contributing/coding-guidelines/)). Note that thesse guidelines have been developed for C# and some adaption may need to happen to fit the different langauges we use. Summaries of the most important parts follow below as well as a cheat sheet.

### Indentation

Use tabs for indentation. Set your text-editor to show them as 8 spaces to incentivise little nesting of loops.

### Where to put braces

| Statement                   | Brace position |
|-----------------------------|----------------|
| Namespace                   | same line      |
| Type                        | same line      |
| Method (including ctor)     | **new line**       |
| Properties                  | same line      |
| Control blocks (if, for...) | same line      |
| Anonymous types and methods | same line      |

### File headers

File headers should inlude file name and a short description of the file as well as the authors of the files. Feel free to use the following template.

'
//
// System.Comment.cs: Handles comments in System files.
//
// Author:
//      John Doe (johndoe@address.com)
//
// License:
//      AFL 3.0 
//
'

### Important sections/cheat sheet
    - [Indentation](https://www.mono-project.com/community/contributing/coding-guidelines/#indentation)
    - [Where to put spaces](https://www.mono-project.com/community/contributing/coding-guidelines/#where-to-put-spaces)
    - [Where to put braces](https://www.mono-project.com/community/contributing/coding-guidelines/#where-to-put-braces)
    - [Multiline parameter](https://www.mono-project.com/community/contributing/coding-guidelines/#multiline-parameters)
    - [Whitespace for clarity](https://www.mono-project.com/community/contributing/coding-guidelines/#use-whitespace-for-clarity)
    - [Multiline comments](https://www.mono-project.com/community/contributing/coding-guidelines/#multiline-comments)
    - [Casing](https://www.mono-project.com/community/contributing/coding-guidelines/#casing)

