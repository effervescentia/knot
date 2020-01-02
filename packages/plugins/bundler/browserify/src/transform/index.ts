// tslint:disable: no-expression-statement
import KnotCompiler, { isKnot } from '@knot/compiler';
import stream from 'stream';
import through, { FlushCallback, TransformCallback } from 'through2';
import nodeTransformer from './node';

function flushTransformed(compiler: KnotCompiler, file: string): FlushCallback {
  return async function(next: TransformCallback): Promise<void> {
    try {
      await compiler.add(file);
      await compiler.awaitModule(file);

      const compiled = await compiler.generate(file);

      if (compiled) {
        this.push(nodeTransformer(compiled, compiler.options));
      }

      this.push(null);
      next();
    } catch (e) {
      next(e);
    }
  };
}

function transformFile(
  compiler: KnotCompiler
): (file: string) => stream.Transform {
  return file => {
    if (isKnot(file)) {
      return through((_, __, next) => next(), flushTransformed(compiler, file));
    }

    return through();
  };
}

export default transformFile;
