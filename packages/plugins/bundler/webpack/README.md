# @knot/webpack-plugin

[![standard-readme compliant](https://img.shields.io/badge/standard--readme-OK-green.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

> A webpack plugin to automatically handle parsing and transforming knot code.

Integrate knot into your existing webpack build easily using this plugin to automatically inject everything from
module resolution to parallelized transcompilation.

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
yarn global add @knot/webpack-plugin
```

## Usage

In order to use knot modules with webpack only requires 2 pieces of configuration.

1. Add `'.kn'` to list of extensions to consider when resolving modules
1. Add an instance of the `KnotPlugin` to the list of plugins

```ts
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

There is also [a more robust example](https://github.com/effervescentia/knot/tree/master/examples/webpack-react).

## Maintainers

[@effervescentia](https://github.com/effervescentia)

## Contributing

PRs accepted.

Small note: If editing the README, please conform to the [standard-readme](https://github.com/RichardLitt/standard-readme) specification.

## License

MIT © 2019 Ben Teichman
