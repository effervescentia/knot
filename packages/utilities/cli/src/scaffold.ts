import gulp from 'gulp';
import conflict from 'gulp-conflict';
import rename from 'gulp-rename';
import template from 'gulp-template';
import path from 'path';

import { BundlerType, FrameworkType, INTERPOLATION_PATTERN } from './contants';

export type ScaffoldOptions = {
  isHTTPS: boolean;
  projectName: string;
  frameworkType: FrameworkType;
  bundlerType: BundlerType;
  targetDir: string;
};

export const scaffoldProject = async ({
  isHTTPS,
  projectName,
  frameworkType,
  bundlerType,
  targetDir
}: ScaffoldOptions) => {
  const templatesDir = path.resolve(__dirname, '../..', 'templates');
  const commonDir = path.resolve(templatesDir, 'common');
  const httpsDir = path.resolve(templatesDir, 'https');
  const frameworkDir = path.resolve(templatesDir, frameworkType);
  const bundlerDir = path.resolve(templatesDir, bundlerType);

  const [
    reactPluginVersion,
    vuePluginVersion,
    webpackPluginVersion,
    rollupPluginVersion,
    browserifyPluginVersion
  ] = await Promise.all([
    '@knot/react-plugin',
    '@knot/vue-plugin',
    '@knot/webpack-plugin',
    '@knot/rollup-plugin',
    '@knot/browserify-plugin'
  ]);

  gulp
    .src(
      [commonDir, frameworkDir, bundlerDir, ...(isHTTPS ? [httpsDir] : [])].map(
        dir => `${dir}/**`
      )
    )
    .pipe(
      rename(file => {
        if (file.basename.startsWith('_')) {
          file.basename = file.basename.slice(1);
        }
      })
    )
    .pipe(
      template(
        {
          isHTTPS,
          projectName,
          bundlerType,
          frameworkType,
          pluginVersions: {
            react: reactPluginVersion,
            vue: vuePluginVersion,
            browserify: browserifyPluginVersion,
            webpack: webpackPluginVersion,
            rollup: rollupPluginVersion
          }
        },
        { interpolate: INTERPOLATION_PATTERN }
      )
    )
    .pipe(conflict(targetDir))
    .pipe(gulp.dest(targetDir));
};
