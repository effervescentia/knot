/* eslint-disable @typescript-eslint/camelcase */
import { JSXPlugin, PluginError } from '@knot/plugin-utils';
import Vue from 'vue';

import createElement from './element';
import StateFactory from './state';
import { VueComponent, VueElement } from './types';

const Plugin: JSXPlugin<string | VueComponent, VueElement> = {
  createElement: (element, rawProps, ...children) => factory => {
    const props = rawProps || {};

    if (props.$$_state) {
      console.log('state', props.$$_state);
    }

    if (typeof element === 'function') {
      return element(props)(factory);
    }

    return createElement(factory, element, props, children);
  },

  createFragment: _ => {
    throw new PluginError('Vue.js does not support fragments');
  },

  render: (app, id) =>
    new Vue({
      el: `#${id}`,
      render(createElement) {
        return app(createElement);
      }
    }),

  withState: (createState, component: VueComponent) => {
    const observable = Vue.observable({});
    const state = createState(new StateFactory(observable));

    return props => createElement =>
      component({
        ...props,
        $$_state: {
          ...props?.$$_state,
          ...state.get()
        }
      })(createElement);
  }
};

export default Plugin;
