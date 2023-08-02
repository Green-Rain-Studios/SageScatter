# SageScatter
Layout and Scatter related helper tools for Unreal Engine

## Overview
This plugin has some layout tools that have developed over time for inhouse use. Ported over first from a blueprint only repo, and then from an older codebase into a formal plugin. The repo will be updated as more stuff is ported into the plugin.

## Documentation
*Pending*

## Versioning
Versions will follow the usual `major.minor.fix` versioning system for tagging. Tagging will be done on the main branch.

### v1
This plugin will be considered to be at version 1 when it reaches parity with the internal tooling that is being ported.

- [x] Generic spline placement actor
- [ ] #1
- [ ] #2

## Contributing
All PR's welcome from forks!
### PR Rules
- Must use git lfs for binary files (default [`.gitattributes`](https://github.com/Green-Rain-Studios/SageScatter/blob/main/.gitattributes) file already exists)
- Must be a PR to the `dev` branch
- If creating a new actor, MUST NOT be only a c++ class. Subclass to a blueprint and follow good folder structure rules

## Contributors
- Aashish Vasudevan - [aashish-greenrain](https://github.com/aashish-greenrain)

## License
Licensed under the MIT License, see the [LICENSE](https://github.com/Green-Rain-Studios/SageScatter/blob/main/LICENSE) for details.