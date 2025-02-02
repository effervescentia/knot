/**
 * incremental engine types
 */

// 1. Central State

interface State {
  sources: Record<string, string>
}

// 2. Typed Pipeline

class Engine { }

abstract class Pipeline<In, Out> {
  abstract process(input: In): Promise<Out>;
  
  chain<Result>(pipe: Pipeline<Out, Result>): Pipeline<In, Result> {
    return new ChainPipeline((input) => this.process(input).then(output => pipe.process(output)))
  }
}

class ChainPipeline<In, Out> extends Pipeline<In, Out> {
  constructor(private readonly transform: (input: In) => Promise<Out>) {
    super()
  }

  async process(input: In): Promise<Out> {
    return this.transform(input)
  }
}
