# Changelog

<!-- INJECT CHANGELOG HERE -->

## [`@knot/compiler-v1.3.0`](https://github.com/effervescentia/knot/releases/tag/@knot/compiler-v1.3.0), [`@knot/webpack-plugin-v1.3.0`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-plugin-v1.3.0), [`@knot/todomvc-example-v1.0.0`](https://github.com/effervescentia/knot/releases/tag/@knot/todomvc-example-v1.0.0), [`@knot/webpack-react-example-v1.0.0`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-react-example-v1.0.0) (2019-9-17)

### `@knot/compiler`


#### :sparkles: New Features

- **encoding**: support unicode characters in source code (#75) ([754a6162](https://github.com/effervescentia/knot/commit/754a6162377d6a5c8264f70a830854e98352fef1))

### `@knot/webpack-plugin`


#### :link: Dependency Updates

- **automatic**: upgrade `@knot/compiler` from `v1.2.8` -> `v1.3.0`

### `@knot/todomvc-example`

:rocket: **Initial Release** :rocket:

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/webpack-plugin` from `v1.2.9` -> `v1.3.0`

### `@knot/webpack-react-example`

:rocket: **Initial Release** :rocket:

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/webpack-plugin` from `v1.2.9` -> `v1.3.0`

## [`@knot/webpack-plugin-v1.2.9`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-plugin-v1.2.9), [`@knot/todomvc-example-v1.0.0`](https://github.com/effervescentia/knot/releases/tag/@knot/todomvc-example-v1.0.0), [`@knot/webpack-react-example-v1.0.0`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-react-example-v1.0.0) (2019-9-17)

### `@knot/webpack-plugin`


#### :bug: Bug Fixes

- allow smarter override for knot binary (#73) ([e7be7b36](https://github.com/effervescentia/knot/commit/e7be7b3698dc26621b05605c7a94bec0bc1a2e57))

### `@knot/todomvc-example`

:rocket: **Initial Release** :rocket:

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/webpack-plugin` from `v1.2.8` -> `v1.2.9`

### `@knot/webpack-react-example`

:rocket: **Initial Release** :rocket:

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/webpack-plugin` from `v1.2.8` -> `v1.2.9`

## [`@knot/compiler-v1.2.8`](https://github.com/effervescentia/knot/releases/tag/@knot/compiler-v1.2.8), [`@knot/cli-v1.1.2`](https://github.com/effervescentia/knot/releases/tag/@knot/cli-v1.1.2), [`@knot/webpack-plugin-v1.2.8`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-plugin-v1.2.8) (2019-9-7)

### `@knot/compiler`

#### :bug: Bug Fixes

- **binary**: use placeholder to install binary properly ([e3638403](https://github.com/effervescentia/knot/commit/e36384034dea300cb31225e51b7c2ffcb05aaa03))

### `@knot/cli`

#### :bug: Bug Fixes

- **binary**: use placeholder to install binary properly ([e3638403](https://github.com/effervescentia/knot/commit/e36384034dea300cb31225e51b7c2ffcb05aaa03))

### `@knot/webpack-plugin`

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/compiler` from `v1.2.7` -> `v1.2.8`

## [`@knot/cli-v1.1.1`](https://github.com/effervescentia/knot/releases/tag/@knot/cli-v1.1.1) (2019-9-5)

### `@knot/cli`

#### :bug: Bug Fixes

- include dotfiles for project generation ([ba5b4397](https://github.com/effervescentia/knot/commit/ba5b43975f2e4bb25a51c2f0a4a9a99065899e46))

## [`@knot/compiler-v1.2.7`](https://github.com/effervescentia/knot/releases/tag/@knot/compiler-v1.2.7), [`@knot/cli-v1.1.0`](https://github.com/effervescentia/knot/releases/tag/@knot/cli-v1.1.0), [`@knot/browser-plugin-v1.2.1`](https://github.com/effervescentia/knot/releases/tag/@knot/browser-plugin-v1.2.1), [`@knot/webpack-plugin-v1.2.7`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-plugin-v1.2.7) (2019-9-5)

### `@knot/compiler`

#### :bug: Bug Fixes

- **pkg**: fix installing compiler binaries ([b0a837b5](https://github.com/effervescentia/knot/commit/b0a837b5e8317c8efa8d4ffcd59dd409d224acdd))
- **pkg**: install binaries in production but not locally ([4c384747](https://github.com/effervescentia/knot/commit/4c38474705b66b3d6e5c263b686d90d624f32837))

### `@knot/cli`

#### :sparkles: New Features

- **https**: add default support for local https ([e361c327](https://github.com/effervescentia/knot/commit/e361c3276a4671b98a7714562226d86e3127d9dd))
#### :bug: Bug Fixes

- add missing template folder ([3d37ef11](https://github.com/effervescentia/knot/commit/3d37ef1146b3b391d954280ac35b002e4e30aeb4))

### `@knot/browser-plugin`

#### :bug: Bug Fixes

- **pkg**: include utils module in released package ([3ddf4fb1](https://github.com/effervescentia/knot/commit/3ddf4fb14b43c38ae7da8d8a6d5ecafc2fb12a90))

### `@knot/webpack-plugin`

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/browser-plugin` from `v1.2.0` -> `v1.2.1`
- **automatic**: upgrade `@knot/compiler` from `v1.2.6` -> `v1.2.7`

## [`@knot/compiler-v1.2.6`](https://github.com/effervescentia/knot/releases/tag/@knot/compiler-v1.2.6), [`@knot/webpack-plugin-v1.2.6`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-plugin-v1.2.6) (2019-9-5)

### `@knot/compiler`

#### :bug: Bug Fixes

- **pkg**: should release with platform-specific artifacts ([50405cf5](https://github.com/effervescentia/knot/commit/50405cf5d455c7cd648e0755e1f2dd43821b9455))

### `@knot/webpack-plugin`

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/compiler` from `v1.2.5` -> `v1.2.6`

## [`@knot/compiler-v1.2.5`](https://github.com/effervescentia/knot/releases/tag/@knot/compiler-v1.2.5), [`@knot/webpack-plugin-v1.2.5`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-plugin-v1.2.5) (2019-9-4)

### `@knot/compiler`

#### :bug: Bug Fixes

- **pkg**: build npm release of compiler properly ([8129db9b](https://github.com/effervescentia/knot/commit/8129db9bd56f65a748366aabdb3784cbb1524e46))

### `@knot/webpack-plugin`

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/compiler` from `v1.2.4` -> `v1.2.5`

## [`@knot/compiler-v1.2.4`](https://github.com/effervescentia/knot/releases/tag/@knot/compiler-v1.2.4), [`@knot/webpack-plugin-v1.2.4`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-plugin-v1.2.4) (2019-9-3)

### `@knot/compiler`

#### :bug: Bug Fixes

- **pkg**: remove prepublishOnly lifecycle hook from compiler ([009fef73](https://github.com/effervescentia/knot/commit/009fef73da1596aab6159fe7389165bee3ad1959))

### `@knot/webpack-plugin`

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/compiler` from `v1.2.3` -> `v1.2.4`

## [`@knot/compiler-v1.2.3`](https://github.com/effervescentia/knot/releases/tag/@knot/compiler-v1.2.3), [`@knot/webpack-plugin-v1.2.3`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-plugin-v1.2.3) (2019-9-3)

### `@knot/compiler`

#### :bug: Bug Fixes

- **pkg**: generate artifacts for release correctly ([431a19c2](https://github.com/effervescentia/knot/commit/431a19c2d38c0010b2346ef479412923f6807288))

### `@knot/webpack-plugin`

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/compiler` from `v1.2.2` -> `v1.2.3`

## [`@knot/compiler-v1.2.2`](https://github.com/effervescentia/knot/releases/tag/@knot/compiler-v1.2.2), [`@knot/webpack-plugin-v1.2.2`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-plugin-v1.2.2) (2019-9-3)

### `@knot/compiler`

#### :bug: Bug Fixes

- **npm**: include compiled binaries in compiler package ([04af0605](https://github.com/effervescentia/knot/commit/04af06057188230e4cfce1a3a9c85564674838a9))

### `@knot/webpack-plugin`

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/compiler` from `v1.2.1` -> `v1.2.2`

## [`@knot/compiler-v1.2.1`](https://github.com/effervescentia/knot/releases/tag/@knot/compiler-v1.2.1), [`@knot/webpack-plugin-v1.2.1`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-plugin-v1.2.1) (2019-8-31)

### `@knot/compiler`

#### :bug: Bug Fixes

- **npm**: fix compiler package to include compiled binaries ([c0bb5d0c](https://github.com/effervescentia/knot/commit/c0bb5d0cecf0c9912c122a698e4b2e8c72932634))

### `@knot/webpack-plugin`

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/compiler` from `v1.2.0` -> `v1.2.1`

## [`@knot/compiler-v1.2.0`](https://github.com/effervescentia/knot/releases/tag/@knot/compiler-v1.2.0), [`@knot/webpack-plugin-v1.2.0`](https://github.com/effervescentia/knot/releases/tag/@knot/webpack-plugin-v1.2.0), [`@knot/browser-plugin-v1.2.0`](https://github.com/effervescentia/knot/releases/tag/@knot/browser-plugin-v1.2.0), [`@knot/jss-plugin-v1.2.0`](https://github.com/effervescentia/knot/releases/tag/@knot/jss-plugin-v1.2.0), [`@knot/react-plugin-v1.2.0`](https://github.com/effervescentia/knot/releases/tag/@knot/react-plugin-v1.2.0), [`@knot/cli-v1.0.0`](https://github.com/effervescentia/knot/releases/tag/@knot/cli-v1.0.0) (2019-08-30)

### `@knot/compiler`

#### :sparkles: New Features

- **release**: align versioning across all packages ([ec7ce098](https://github.com/effervescentia/knot/commit/ec7ce0984e26fa6879029537f76980d92573f69a))

### `@knot/webpack-plugin`

#### :sparkles: New Features

- **release**: align versioning across all packages ([ec7ce098](https://github.com/effervescentia/knot/commit/ec7ce0984e26fa6879029537f76980d92573f69a))

#### :link: Dependency Updates

- **automatic**: upgrade `@knot/browser-plugin` from `v1.1.0` -> `v1.2.0`
- **automatic**: upgrade `@knot/compiler` from `v1.1.1` -> `v1.2.0`
- **automatic**: upgrade `@knot/jss-plugin` from `v1.1.0` -> `v1.2.0`
- **automatic**: upgrade `@knot/react-plugin` from `v1.1.1` -> `v1.2.0`

### `@knot/browser-plugin`

#### :sparkles: New Features

- **release**: align versioning across all packages ([ec7ce098](https://github.com/effervescentia/knot/commit/ec7ce0984e26fa6879029537f76980d92573f69a))

<details><summary>Additional Details</summary><p>

#### :muscle: Refactors

- extract functionality into separate files and functions ([c40fa687](https://github.com/effervescentia/knot/commit/c40fa6873c2678584680c274d6895ccf042a38ae))

</p></details>

### `@knot/jss-plugin`

#### :sparkles: New Features

- **release**: align versioning across all packages ([ec7ce098](https://github.com/effervescentia/knot/commit/ec7ce0984e26fa6879029537f76980d92573f69a))

<details><summary>Additional Details</summary><p>

#### :muscle: Refactors

- extract functionality into separate files and functions ([c40fa687](https://github.com/effervescentia/knot/commit/c40fa6873c2678584680c274d6895ccf042a38ae))

</p></details>

### `@knot/react-plugin`

#### :sparkles: New Features

- **release**: align versioning across all packages ([ec7ce098](https://github.com/effervescentia/knot/commit/ec7ce0984e26fa6879029537f76980d92573f69a))

### `@knot/cli`

#### :sparkles: New Features

- **release**: align versioning across all packages ([ec7ce098](https://github.com/effervescentia/knot/commit/ec7ce0984e26fa6879029537f76980d92573f69a))

<details><summary>Additional Details</summary><p>

#### :muscle: Refactors

- extract functionality into separate files and functions ([c40fa687](https://github.com/effervescentia/knot/commit/c40fa6873c2678584680c274d6895ccf042a38ae))

</p></details>