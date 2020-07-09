declare module 'jss-preset-default' {
  import { JSSOptions } from 'jss';

  const jssPreset: () => Partial<JSSOptions>;

  export default jssPreset;
}
