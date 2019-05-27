declare module 'gulp-template' {
  import * as stream from 'stream';

  function gulpTemplate(
    data: any,
    options?: _.TemplateOptions
  ): stream.Transform;

  export = gulpTemplate;
}
