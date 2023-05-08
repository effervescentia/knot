import { extendScripts } from '../../../scripts';
import { configureBuild } from '../../../scripts/build';

export default extendScripts({
  build: configureBuild(true)
});
