import KnotCompiler, {
  isKnot,
  OptionOverrides,
  resolveLibrary
} from '@knot/compiler';
import nodeResolve from 'resolve';
import { Plugin, ResolveIdResult } from 'rollup';

function knotRollupPlugin(options: OptionOverrides = {}): Plugin {
  const compiler = new KnotCompiler(options);

  return {
    name: 'knot',

    async buildStart() {
      await compiler.awaitReady();
    },

    async resolveId(id, importer) {
      const resolved = resolveLibrary(id, compiler.options);
      if (resolved) {
        return this.resolve(resolved, importer, { skipSelf: true }).then(
          result => {
            if (!result) {
              return { id: resolved };
            }

            return result;
          }
        );
      }

      if (!id.startsWith('@knot/') && isKnot(importer)) {
        return new Promise<ResolveIdResult>((resolve, reject) =>
          nodeResolve(
            id,
            {
              extensions: ['.kn']
            },
            (err, result) => (err ? reject(err) : resolve(result))
          )
        );
      }

      return null;
    },

    async transform(_, id) {
      if (isKnot(id)) {
        await compiler.add(id);
        await compiler.awaitModule(id);

        const compiled = await compiler.generate(id);
        if (compiled) {
          return {
            code: compiled,
            map: null
          };
        }
      }

      return null;
    },

    async buildEnd() {
      await compiler.close();
    }
  };
}

export default knotRollupPlugin;
