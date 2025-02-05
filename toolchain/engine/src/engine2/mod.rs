mod input;
mod pipeline;
mod plan;

use pipeline::{Execute, Identity, Transform};

/*
phases

1. empty
2. collect inputs (entry, globs, libraries)
3. parse (optional discovery)
4. link
5. analyze
6. generate
*/

#[derive(Clone, Copy)]
pub struct Library;

#[derive(Clone, Copy)]
pub struct Context;

pub struct State;

pub struct Engine(Context);

impl Engine {
    pub fn new(context: Context) -> Self {
        Self(context)
    }

    pub fn input() -> input::Builder<Identity<Context>, Library> {
        input::Builder::new()
    }

    pub fn plan() -> plan::Builder<Identity<(State, Vec<String>)>> {
        plan::Builder::new()
    }

    pub fn execute<R, T1, T2, S>(
        &self,
        plan: &plan::Builder<T1>,
        input: &input::Builder<T2, Library>,
    ) -> (State, R)
    where
        T1: Transform<In = (State, Vec<String>), Out = (State, R)>,
        T2: Transform<In = Context, Out = input::Input<S, Library>>,
    {
        let state = State;
        let input::Input { source, libraries } = input.execute(self.0);

        plan.execute((state, vec![]))
    }

    pub fn incremental<R, T, U>(
        &self,
        state: State,
        plan: &plan::Builder<T>,
        targets: U,
    ) -> (State, R)
    where
        T: Transform<In = (State, Vec<String>), Out = (State, R)>,
        U: AsRef<[String]>,
    {
        let context = self.0;

        plan.execute((state, targets.as_ref().to_vec()))
    }
}

#[cfg(test)]
mod tests {
    use super::{pipeline::Peek, Context, Engine, Library};

    #[test]
    fn one_off_execution() {
        let context = Context;
        let engine = Engine::new(context);
        let input = Engine::input().from_entry("entry");
        let pipeline = Engine::plan();

        let (_, result) = engine.execute(&pipeline, &input);
    }

    #[test]
    fn incremental_execution() {
        let context = Context;
        let engine = Engine::new(context);
        let input = Engine::input().from_entry("entry");
        let pipeline = Engine::plan();

        let (mut state, _) = engine.execute(&pipeline, &input);

        loop {
            let changed = vec![];

            (state, _) = engine.incremental(state, &pipeline, changed);
        }
    }

    #[test]
    fn build_pipeline() {
        let plan = Engine::plan()
            .parse()
            .peek(|_| ())
            .link()
            .peek(|_| ())
            .analyze()
            .peek(|_| ())
            .generate(())
            .peek(|_| ());
    }

    #[test]
    fn format_pipeline() {
        let plan = Engine::plan() //
            .parse()
            .peek(|_| ())
            .format()
            .peek(|_| ());
    }
}
