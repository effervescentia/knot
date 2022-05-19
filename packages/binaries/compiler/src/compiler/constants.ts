import { Options, Target } from '../types';

export const MAX_ATTEMPTS = 10;
export const INFINITE_ATTEMPTS = 100;
export const ATTEMPT_TIMEOUT = 1000;

export const DEFAULT_OPTIONS: Options = {
  cwd: process.cwd(),
  target: Target.JAVASCRIPT_ES6,
  plugins: {
    jsx: '@knot/react-plugin',
    platform: '@knot/browser-plugin',
    style: '@knot/jss-plugin'
  }
};
