import execa from 'execa';

export const cli = (...args: string[]) =>
  execa('node', ['index.js', ...args], {
    cwd: process.cwd()
  });
