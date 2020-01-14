import { filterKeys, groupEntries, mapKeys } from '@knot/common';
import { PropsType } from '@knot/plugin-utils';

const IGNORE_PROPS = ['className'];

const GROUP_PREDICATES = {
  attrs: ([, value]) => typeof value !== 'function',
  on: ([key, value]) => typeof value === 'function' && key.startsWith('on')
};

const createNodeData = (props: PropsType) => {
  const staticClass = props.className;

  const filteredProps = filterKeys(props, key => !IGNORE_PROPS.includes(key));

  const groupedProps = groupEntries(filteredProps, GROUP_PREDICATES);

  const eventHandlers: Record<string, Function> = mapKeys(
    groupedProps.on,
    key => key.substr(2).toLowerCase()
  );

  return {
    staticClass,
    attrs: groupedProps.attrs,
    on: eventHandlers
  };
};

export default createNodeData;
