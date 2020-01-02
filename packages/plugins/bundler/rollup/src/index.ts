// tslint:disable: no-expression-statement
import KnotCompiler, { isKnot, Options, resolveLibrary } from '@knot/compiler';
// import KnotCompiler, { isKnot, Options } from '@knot/compiler';
import nodeResolve from '@rollup/plugin-node-resolve';
import { Plugin } from 'rollup';

function knotRollupPlugin(options: Partial<Options> = {}): Plugin {
  const compiler = new KnotCompiler(options);
  const knotResolver = nodeResolve({ extensions: ['.kn'] });

  return {
    name: 'knot',

    async buildStart() {
      await compiler.awaitReady();
    },

    async resolveId(id, importer) {
      const resolved = resolveLibrary(id, compiler.options);
      if (resolved) {
        return this.resolve(resolved, importer, { skipSelf: true });
      }

      if (!id.startsWith('@knot/') && isKnot(importer)) {
        return knotResolver.resolveId.call(this, id, importer, {
          skipSelf: true
        });
      }

      return null;
    },

    async transform(_, id) {
      if (isKnot(id)) {
        await compiler.add(id);
        await compiler.awaitModule(id);

        const compiled = await compiler.generate(id);
        if (compiled) {
          console.log(compiled);
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
