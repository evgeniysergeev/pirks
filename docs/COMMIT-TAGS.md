- Separate subject from body with a blank line
- Limit the subject line to 72 (maximum 80) characters
- Capitalise the subject line
- Do not end the subject line with a period
- Wrap the body at 72 (maximum 80) characters
- Use the body to explain what and why vs how


Commits can have this tags:
| Tag        | Description
|:-----------|:--
| [feature]  | New feature for the user, not a new feature for build scripts
| [fix]       | Bug fix for the user, not a fix to a build script
| [docs]     | Changes to the documentation
| [tests]    | Adding missing tests, refactoring tests; not a production code change
| [style]    | Formatting, missing semi colons, etc; not a production code change
| [refactor] | Refactoring production code, eg. renaming a variable
| [internal] | Changes in sources, utilities, devops or something not visible to use

Commit scope can be:
| Scope      | Description
|:-----------|:--
| (input)    | Related to capture and send input (mouse, keyboard, gamepad and so on)
| (audio)    | Related to audio
| (video)    | Related to video capture, encode, decode and so on.
| (network)  | Related to networking
| (ui)       | Related to client ui
| (build)    | Related to build, change version, upload release build etc.

commit message will be:
```
[type](scope) Commit title (#<issue number>)
```

For example:
```
[feature] Create main function (#1)
```
or just:
```
Create main function (#1)
```
```
Update README.md
```
