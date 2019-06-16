import { series } from 'nps-utils';

export function configureBuild(buildModule?: boolean): object {
  return {
    default: {
      description: 'clean build directory then generate all modules',
      script: series.nps(
        'clean',
        'build.main',
        ...(buildModule ? ['build.module'] : [])
      )
    },
    main: {
      description: 'generate an es5 compatible node.js module',
      script: 'tsc -p tsconfig.json'
    },
    module: {
      description: 'generate an esnext module',
      script: 'tsc -p tsconfig.module.json'
    }
  };
}

export default configureBuild();
