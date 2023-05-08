import { JSXPlugin, PluginError, PropsType, Style } from '@knot/plugin-utils';
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

  bindStyle<P extends PropsType>(
    component: ((props: P) => VueComponent) | string,
    _style: Style
  ): (props: P) => VueComponent {
    return component as any;
  },
};

export default Plugin;
