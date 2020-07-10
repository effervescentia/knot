# knot

[![npm](https://img.shields.io/npm/v/@knot/cli?style=flat-square)](http://npm.im/@knot/cli)
[![code style: prettier](https://img.shields.io/badge/code_style-prettier-ff69b4.svg?style=flat-square)](https://github.com/prettier/prettier)
[![lerna](https://img.shields.io/badge/maintained%20with-lerna-cc00ff.svg?style=flat-square)](https://lerna.js.org/)
[![semantic-release](https://img.shields.io/badge/%20%20%F0%9F%93%A6%F0%9F%9A%80-semantic--release-e10079.svg?style=flat-square)](https://github.com/semantic-release/semantic-release)
[![Commitizen friendly](https://img.shields.io/badge/commitizen-friendly-brightgreen.svg?style=flat-square)](http://commitizen.github.io/cz-cli/)
[![standard-readme compliant](https://img.shields.io/badge/standard--readme-OK-green.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

[![CircleCI](https://img.shields.io/circleci/build/gh/effervescentia/knot?style=flat-square&token=c6d265c2c3ae9fea01043c75299974616b6498b0)](https://circleci.com/gh/effervescentia/knot)
[![codecov](https://codecov.io/gh/effervescentia/knot/branch/master/graph/badge.svg)](https://codecov.io/gh/effervescentia/knot)

> A programming language, its compiler and supporting tooling.

A functional language with touches of imperative sugar; designed to make building interfaces simple and portable.
Inspired by [`react`](https://reactjs.org), [`redux`](https://redux.js.org), [`elm`](https://elm-lang.org) and many others.

## Table of Contents

- [Usage](#usage)
- [Examples](#examples)
- [Packages](#packages)
- [Editors](#editors)
- [Maintainers](#maintainers)
- [Contributing](#contributing)
- [License](#license)

## ❓ Usage

Documentation for the Knot Language can be found [on gitbook](https://knot.gitbook.io/language).
Otherwise, check the `README` of the package for specific documentation.

## 🎁 Examples

This repository contains a number of example projects to help you get started quickly.

- [`@knot/webpack-react-example`](https://github.com/effervescentia/knot/tree/master/examples/webpack-react): An example using [`webpack`](https://webpack.js.org/) and [`react`](https://reactjs.org)
- [`@knot/webpack-vue-example`](https://github.com/effervescentia/knot/tree/master/examples/webpack-vue): An example using [`webpack`](https://webpack.js.org/) and [`vue`](https://vuejs.org/)
- [`@knot/browserify-react-example`](https://github.com/effervescentia/knot/tree/master/examples/browserify-react): An example using [`browserify`](http://browserify.org/) and [`react`](https://reactjs.org)
- [`@knot/rollup-react-example`](https://github.com/effervescentia/knot/tree/master/examples/rollup-react): An example using [`rollup`](https://rollupjs.org/) and [`react`](https://reactjs.org)
- [`@knot/todomvc-example`](https://github.com/effervescentia/knot/tree/master/examples/todomvc): An example based on [TodoMVC](http://todomvc.com/)

## 📦 Packages

Plugins and tools for developing with `knot`.

- [`@knot/cli`](http://npm.im/@knot/cli): A project generator for creating new `knot` projects
- [`@knot/compiler`](http://npm.im/@knot/compiler): A module which downloads and wraps platform-specific version of the compiler

### 🗜 Build Tools

These packages are meant to integrate with existing tools and workflows.

- [`@knot/webpack-plugin`](http://npm.im/@knot/webpack-plugin): A [`webpack`](https://webpack.js.org/) plugin to support `knot` files
- [`@knot/browserify-plugin`](http://npm.im/@knot/browserify-plugin): A [`browserify`](http://browserify.org/) plugin to support `knot` files
- [`@knot/rollup-plugin`](http://npm.im/@knot/rollup-plugin): A [`rollup`](https://rollupjs.org/) plugin to support `knot` files

### ☢️ Runtime

These packages are intended to be used by `knot` to provide runtime functionality.

#### Platform

Plugins to provide a platform runtime.

- [`@knot/browser-plugin`](http://npm.im/@knot/browser-plugin): A `platform` runtime plugin for the web

#### Style

Plugins to provide a styling runtime.

- [`@knot/jss-plugin`](http://npm.im/@knot/jss-plugin): A `style` runtime plugin for [`JSS`](https://cssinjs.org/)

#### JSX

Plugins to provide a JSX runtime.

- [`@knot/react-plugin`](http://npm.im/@knot/react-plugin): A `jsx` runtime plugin for [`react`](https://reactjs.org)
- [`@knot/vue-plugin`](http://npm.im/@knot/vue-plugin): A `jsx` runtime plugin for [`vue`](https://vuejs.org/)

## 💾 Editors

Integrate with the IDE of your choice.

### VS Code

- [`knot-vscode-plugin`](https://marketplace.visualstudio.com/items?itemName=knot-lang.knot-vscode-plugin): Provides syntax highlighting

## Maintainers

[@effervescentia](https://github.com/effervescentia)

## Contributing

PRs accepted.

Small note: If editing the README, please conform to the [standard-readme](https://github.com/RichardLitt/standard-readme) specification.

## License

MIT © 2019 Ben Teichman
