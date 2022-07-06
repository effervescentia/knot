import { JSXPlugin, PluginError } from '@knot/plugin-utils';
import Vue from 'vue';

import { VueComponent, VueElement } from './types';

const Plugin: JSXPlugin<VueElement, VueComponent> = {
  createTag: (_tag, _props, ..._children) => null as any,

  createFragment: () => {
    throw new PluginError('Vue.js does not yet support fragments');
  },

  render: (_app, id) =>
    new Vue({
      el: `#${id}`,
    }),
};

export default Plugin;
