import { JSXPlugin, PluginError, STATE_MAP_KEY } from '@knot/plugin-utils';
import Vue from 'vue';

import createData from './data';
import StateFactory from './state';
import { ElementFactory, VueComponent, VueElement } from './types';

const wrapFactory = (factory: ElementFactory) => {
  const render = (element: VueElement | string | number | undefined | null) => {
    if (!element || typeof element !== 'object') {
      return element;
    }

    const data = createData(element.props);
    const children = element.children.map(render);

    return factory(element.type, data, children);
  };

  return render;
};

const Plugin: JSXPlugin<VueComponent, VueElement> = {
  createComponent: (_name, component) => ({
    inheritAttrs: false,
    render(factory) {
      const element = component({
        ...this.$attrs,
        [STATE_MAP_KEY]: this.$options[STATE_MAP_KEY]?.get()
      });
      const render = wrapFactory(factory);

      return render(element);
    }
  }),

  createElement: (element, props, ...children) => ({
    type: element,
    props: props || {},
    children
  }),

  createFragment: () => {
    throw new PluginError('Vue.js does not yet support fragments');
  },

  render: (app, id) =>
    new Vue({
      el: `#${id}`,
      render(factory) {
        const render = wrapFactory(factory);

        return render(app);
      }
    }),

  withState: (createState, component) => ({
    ...component,
    beforeCreate() {
      const state = createState(new StateFactory(() => this.$forceUpdate()));
      this.$options[STATE_MAP_KEY] = state;
    }
  })
};

export default Plugin;
