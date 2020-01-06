import { Options } from '../types';

export const MAX_ATTEMPTS = 10;
export const INFINITE_ATTEMPTS = 100;
export const ATTEMPT_TIMEOUT = 1000;

export const DEFAULT_OPTIONS = {
  compiler: {
    module: 'es6'
  },
  config: process.cwd(),
  debug: false,
  plugins: {
    jsx: '@knot/react-plugin',
    platform: '@knot/browser-plugin',
    style: '@knot/jss-plugin'
  },
  port: 1338
} as Options;
