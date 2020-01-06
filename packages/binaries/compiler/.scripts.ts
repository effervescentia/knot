import { extendScripts } from '../../../scripts';

export default extendScripts({
  docs: {
    description: 'generate documentation',
    script:
      'typedoc --excludeNotExported --excludeExternals --plugin @knot/typedoc-plugin --mode modules --json docs.json src'
  }
});
