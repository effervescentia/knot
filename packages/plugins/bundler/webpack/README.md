# @knot/webpack-plugin

[![npm](https://img.shields.io/npm/v/@knot/webpack-plugin?style=flat-square)](http://npm.im/@knot/webpack-plugin)
[![code style: prettier](https://img.shields.io/badge/code_style-prettier-ff69b4.svg?style=flat-square)](https://github.com/prettier/prettier)
[![lerna](https://img.shields.io/badge/maintained%20with-lerna-cc00ff.svg?style=flat-square)](https://lerna.js.org/)
[![semantic-release](https://img.shields.io/badge/%20%20%F0%9F%93%A6%F0%9F%9A%80-semantic--release-e10079.svg?style=flat-square)](https://github.com/semantic-release/semantic-release)
[![Commitizen friendly](https://img.shields.io/badge/commitizen-friendly-brightgreen.svg?style=flat-square)](http://commitizen.github.io/cz-cli/)
[![standard-readme compliant](https://img.shields.io/badge/standard--readme-OK-green.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

[![CircleCI](https://img.shields.io/circleci/build/gh/effervescentia/knot?style=flat-square&token=c6d265c2c3ae9fea01043c75299974616b6498b0)](https://circleci.com/gh/effervescentia/knot)

> A webpack plugin to automatically handle parsing and transforming knot code.

Integrate knot into your existing webpack build easily using this plugin.

## Table of Contents

- [Install](#install)
- [Usage](#usage)
- [Maintainers](#maintainers)
- [Contributing](#contributing)
- [License](#license)

## Install

```sh
npm install @knot/webpack-plugin
# or
yarn add @knot/webpack-plugin
```

## Usage

In order to use knot modules with webpack only requires 2 pieces of configuration.

1. Add `'.kn'` to the list of extensions to consider when resolving modules
1. Add an instance of the `KnotPlugin` to the list of plugins

```js
// webpack.config.js
import KnotPlugin from '@knot/webpack-plugin';

module.exports = {
  // add these to your existing configuration

  resolve: {
    extensions: ['.kn', '.js']
  },

  plugins: [
    new KnotPlugin({
      // optionally print an exhaustive log of the operation of the compiler
      // debug: true,
    })
  ]
};
```

You will now be able to import a `.kn` file from any normal javascript file.
When running webpack, these files will be automatically converted into native javascript modules.

There is also [a more robust example](https://github.com/effervescentia/knot/tree/master/examples/webpack-react).

## Maintainers

[@effervescentia](https://github.com/effervescentia)

## Contributing

PRs accepted.

Small note: If editing the README, please conform to the [standard-readme](https://github.com/RichardLitt/standard-readme) specification.

## License

MIT © 2019 Ben Teichman
