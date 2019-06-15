declare module 'gulp-conflict' {
  interface Options {
    readonly cwd?: string;
    readonly defaultChoice?: 'y' | 'n' | 'd';
  }

  function conflict(dest: string, opts?: Options): NodeJS.ReadWriteStream;

  export = conflict;
}
