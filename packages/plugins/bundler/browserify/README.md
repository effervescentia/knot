# @knot/browserify-plugin

[![npm](https://img.shields.io/npm/v/@knot/browserify-plugin?style=flat-square)](http://npm.im/@knot/browserify-plugin)
[![code style: prettier](https://img.shields.io/badge/code_style-prettier-ff69b4.svg?style=flat-square)](https://github.com/prettier/prettier)
[![lerna](https://img.shields.io/badge/maintained%20with-lerna-cc00ff.svg?style=flat-square)](https://lerna.js.org/)
[![semantic-release](https://img.shields.io/badge/%20%20%F0%9F%93%A6%F0%9F%9A%80-semantic--release-e10079.svg?style=flat-square)](https://github.com/semantic-release/semantic-release)
[![Commitizen friendly](https://img.shields.io/badge/commitizen-friendly-brightgreen.svg?style=flat-square)](http://commitizen.github.io/cz-cli/)
[![standard-readme compliant](https://img.shields.io/badge/standard--readme-OK-green.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

[![CircleCI](https://img.shields.io/circleci/build/gh/effervescentia/knot?style=flat-square&token=c6d265c2c3ae9fea01043c75299974616b6498b0)](https://circleci.com/gh/effervescentia/knot)
[![codecov](https://codecov.io/gh/effervescentia/knot/branch/master/graph/badge.svg?flag=browserify-plugin)](https://codecov.io/gh/effervescentia/knot)

> A [`browserify`](http://browserify.org/) plugin to automatically handle parsing and transforming [`knot`](https://github.com/effervescentia/knot) code.

Integrate `knot` into your existing `browserify` build easily using this plugin.

## Table of Contents

- [Install](#install)
- [Usage](#usage)
- [Maintainers](#maintainers)
- [Contributing](#contributing)
- [License](#license)

## Install

```sh
npm install @knot/browserify-plugin
# or
yarn add @knot/browserify-plugin
```

## Usage

In order to use `knot` modules with `browserify`, add the plugin to your build command.

```sh
browserify main.kn -p [ @knot/browserify-plugin ] > bundle.js
# optionally print an exhaustive log of the operation of the compiler
browserify main.kn -p [ @knot/browserify-plugin --debug ] > bundle.js
```

### Usage with programmatic API

```js
// browserify.js
import browserify from 'browserify';
import knotify from '@knot/browserify-plugin';

browserify()
  // add the plugin to your exsiting configuration
  .plugin(knotify, {
    // optionally print an exhaustive log of the operation of the compiler
    // debug: true,
  });
```

You will now be able to import a `.kn` file from any normal javascript file.
When running `browserify`, these files will be automatically converted into native javascript modules.

There is also [a more robust example](https://github.com/effervescentia/knot/tree/master/examples/browserify-react).

## Maintainers

[@effervescentia](https://github.com/effervescentia)

## Contributing

PRs accepted.

Small note: If editing the README, please conform to the [standard-readme](https://github.com/RichardLitt/standard-readme) specification.

## License

MIT © 2019 Ben Teichman
