export const INTERPOLATION_PATTERN = /#{([\S\s]+?)}/g;

export enum BundlerType {
  WEBPACK = 'webpack',
  ROLLUP = 'rollup',
  BROWSERIFY = 'browserify'
}

export enum FrameworkType {
  REACT = 'react',
  VUE = 'vue'
}

export const FRAMEWORKS = [FrameworkType.REACT, FrameworkType.VUE];
export const FRAMEWORK_BUNDLERS = {
  [FrameworkType.REACT]: [
    BundlerType.WEBPACK,
    BundlerType.BROWSERIFY,
    BundlerType.ROLLUP
  ],
  [FrameworkType.VUE]: [BundlerType.WEBPACK, BundlerType.ROLLUP]
};
