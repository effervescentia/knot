mod input;
mod pipeline;
mod plan;
mod state;

use input::Source;
use pipeline::{Execute, Identity, Transform};
use state::State;
use std::path::{Path, PathBuf};

/*
phases

1. empty
2. collect inputs (entry, globs, libraries)
3. parse (optional discovery)
4. link
5. analyze
6. generate
*/

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum Library {
    MockA,
    MockB,
    MockC,
}

#[derive(Debug)]
pub struct Context {
    source_dir: PathBuf,
    is_verbose: bool,
}

impl Context {
    pub fn new<T>(source_dir: T, is_verbose: bool) -> Self
    where
        T: AsRef<Path>,
    {
        Self {
            source_dir: source_dir.as_ref().to_path_buf(),
            is_verbose,
        }
    }
}

#[derive(Debug)]
pub struct Scope {
    pub files: Vec<PathBuf>,
    pub follow_imports: bool,
}

#[derive(Clone, Copy)]
pub enum Operation {
    Create,
    Update,
    Delete,
}

pub struct Engine(Context);

impl Engine {
    pub const fn new(context: Context) -> Self {
        Self(context)
    }

    pub const fn plan<'a>() -> plan::Builder<Identity<(State<'a>, ())>> {
        plan::Builder::new()
    }

    pub fn execute<'a, Src, Res, Tx>(
        &'a self,
        plan: &plan::Builder<Tx>,
        input: &input::Input<Src, Library>,
    ) -> (State<'a>, Res)
    where
        Src: Source,
        Tx: Transform<In = (State<'a>, ()), Out = (State<'a>, Res)>,
    {
        let input::Input { source, libraries } = input;
        let scope = source.resolve(&self.0.source_dir);

        let state = State::new(&self.0, scope);

        plan.execute((state, ()))
    }

    pub fn incremental<'a, Res, Tx, Ops>(
        prev: State<'a>,
        plan: &plan::Builder<Tx>,
        operations: Ops,
    ) -> (State<'a>, Res)
    where
        Tx: Transform<In = (State<'a>, ()), Out = (State<'a>, Res)>,
        Ops: AsRef<[(PathBuf, Operation)]>,
    {
        let next = prev.evolve(operations);

        plan.execute((next, ()))
    }
}

#[cfg(test)]
mod tests {
    use super::{input::Input, pipeline::Peek, Context, Engine, Library};

    #[test]
    fn one_off_execution() {
        let context = Context::new("source_dir", false);
        let engine = Engine::new(context);
        let input = Input::from_entry("entry", []);
        let pipeline = Engine::plan();

        let (_, result) = engine.execute(&pipeline, &input);
    }

    #[test]
    #[ignore = "skip temporarily"]
    fn incremental_execution() {
        let context = Context::new("source_dir", false);
        let engine = Engine::new(context);
        let input = Input::from_entry("entry", []);
        let plan = Engine::plan();

        let (mut state, _) = engine.execute(&plan, &input);

        loop {
            let operations = vec![];
            // let next_state = state.evolve(operations);

            (state, _) = Engine::incremental(state, &plan, operations);
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
