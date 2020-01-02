// tslint:disable: no-expression-statement
import KnotCompiler, { isKnot, Options, resolveLibrary } from '@knot/compiler';
import nodeResolve from 'resolve';
import { Plugin, ResolveIdResult } from 'rollup';

function knotRollupPlugin(options: Partial<Options> = {}): Plugin {
  const compiler = new KnotCompiler(options);

  return {
    name: 'knot',

    // tslint:disable-next-line: typedef
    async buildStart() {
      await compiler.awaitReady();
    },

    // tslint:disable-next-line: typedef
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

    // tslint:disable-next-line: typedef
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

    // tslint:disable-next-line: typedef
    async buildEnd() {
      await compiler.close();
    }
  };
}

export default knotRollupPlugin;
