import { filterKeys, groupEntries, mapKeys } from '@knot/common';
import { PropsType } from '@knot/plugin-utils';
import * as Vue from 'vue';

const IGNORE_PROPS = ['className'];

const GROUP_PREDICATES = {
  attrs: ([, value]) => typeof value !== 'function',
  on: ([key, value]) => typeof value === 'function' && key.startsWith('on'),
};

const createData = (props: PropsType = {}): Vue.VNodeData => {
  const filteredProps = filterKeys(props, (key) => !IGNORE_PROPS.includes(key));

  const groupedProps = groupEntries(filteredProps, GROUP_PREDICATES);

  const eventHandlers: Record<string, () => unknown> = mapKeys(
    groupedProps.on,
    (key) => key.substr(2).toLowerCase()
  );

  return {
    staticClass: props.className,
    attrs: groupedProps.attrs,
    on: eventHandlers,
  };
};

export default createData;
