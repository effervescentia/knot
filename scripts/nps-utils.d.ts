// tslint:disable: readonly-array

declare module 'nps-utils' {
  export interface Options {
    readonly helpStyle: 'scripts';
  }

  export function series(...scripts: Readonly<string[]>): string;
  export namespace series {
    export function nps(...tasks: Readonly<string[]>): string;
  }

  interface ConcurrentScript {
    readonly script: string;
    readonly color?: string;
  }

  export function concurrent(
    scripts: Record<string, string | ConcurrentScript>
  ): string;
  export namespace concurrent {
    export function nps(...tasks: Readonly<string[]>): string;
  }

  export function open(args: string): string;

  export function rimraf(args: string): string;
}
