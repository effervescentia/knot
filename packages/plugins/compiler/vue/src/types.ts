import { PropsType } from '@knot/plugin-utils';
import * as Vue from 'vue';

export type ElementFactory = (
  tagName: string | VueComponent,
  data?: Vue.VNodeData,
  children?: Vue.VNode[]
) => Vue.VNode;

export type VueComponent = {
  render(this: Vue, factory: ElementFactory): Vue.VNode;
  beforeCreate?(this: Vue): void;
  inheritAttrs?: boolean;
};

export type VueElement = {
  type: string | VueComponent;
  props: PropsType;
  children: VueElement[];
};
