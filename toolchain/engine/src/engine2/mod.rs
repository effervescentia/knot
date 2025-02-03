mod input;
mod pipeline;
mod plan;

use pipeline::{Identity, Transform};

/*
phases

1. empty
2. collect inputs (entry, globs, libraries)
3. parse (optional discovery)
4. link
5. analyze
6. generate
*/

pub struct Context;

pub struct State;

pub struct Engine(Context);

impl Engine {
    pub fn new(context: Context) -> Self {
        Self(context)
    }

    pub fn input() -> input::Builder<Identity<Context>> {
        input::Builder::new()
    }

    pub fn plan() -> plan::Builder<Identity<(State, Vec<String>)>> {
        plan::Builder::new()
    }

    pub fn execute<R, T1, T2, S>(
        &self,
        plan: &plan::Builder<T1>,
        input: &input::Builder<T2>,
    ) -> (State, R)
    where
        T1: Transform<In = (State, Vec<String>), Out = (State, R)>,
        T2: Transform<In = Context, Out = input::Input<S>>,
    {
        (State, todo!())
    }

    pub fn incremental<R, T>(
        &self,
        state: State,
        plan: &plan::Builder<T>,
        targets: Vec<&str>,
    ) -> (State, R)
    where
        T: Transform<In = (State, Vec<String>), Out = (State, R)>,
    {
        (state, todo!())
    }
}

#[cfg(test)]
mod tests {
    use super::{
        pipeline::{Peek, Transform},
        Context, Engine, State,
    };

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
    fn standard_build_pipeline() {
        let context = Context;
        let engine = Engine::new(context);

        let input = Engine::input()
            .from_entry("entry")
            .peek(|_| ())
            .with_libraries([""])
            .peek(|_| ());

        let pipeline = Engine::plan()
            .parse()
            .peek(|_| ())
            .link()
            .peek(|_| ())
            .analyze()
            .peek(|_| ());

        let (_, result) = engine.execute(&pipeline, &input);
    }
}
