import { Options } from '../types';

export const MAX_ATTEMPTS = 10;
export const INFINITE_ATTEMPTS = 100;
export const ATTEMPT_TIMEOUT = 1000;

export const DEFAULT_OPTIONS = {
  target: 'javascript-es6',
  rootDir: process.cwd(),
  debug: false,
  plugins: {
    jsx: '@knot/react-plugin',
    platform: '@knot/browser-plugin',
    style: '@knot/jss-plugin'
  },
  port: 1338,
  config: '.knot.yml'
} as Options;
