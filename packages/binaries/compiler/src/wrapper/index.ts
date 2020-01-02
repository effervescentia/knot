import { JSX_PLUGIN, PLATFORM_PLUGIN, STYLE_PLUGIN } from '../constants';
import { Options } from '../types';
import CommonWrapper from './common';
import ES6Wrapper from './es6';

function wrapModule(
  code: string,
  { plugins, compiler: { module } }: Options
): string {
  const wrapper = module === 'common' ? CommonWrapper : ES6Wrapper;

  return [
    wrapper.genMainImport(JSX_PLUGIN, plugins.jsx),
    wrapper.genMainImport(STYLE_PLUGIN, plugins.style),
    wrapper.genModuleImport(PLATFORM_PLUGIN, plugins.platform),
    code
  ].join('');
}

export default wrapModule;
