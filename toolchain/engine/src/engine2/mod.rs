mod input;
mod logger;
mod modules;
mod plan;
mod state;

pub use input::Input;
use input::Source;
use kore::pipeline::{Execute, Identity, Transform};
#[cfg(test)]
pub use logger::MemoryLogger;
pub use logger::{Logger, NoopLogger};
pub use plan::{Analyzed, Builder, Linked, Parsed};
pub use state::State;
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
pub struct Context<Log> {
    root_dir: PathBuf,
    logger: Log,
}

impl<Log> Context<Log> {
    pub fn new<T>(root_dir: T, logger: Log) -> Self
    where
        T: AsRef<Path>,
    {
        Self {
            root_dir: root_dir.as_ref().to_path_buf(),
            logger,
        }
    }
}

#[cfg(test)]
impl Context<logger::NoopLogger> {
    pub fn mock() -> Self {
        Self::new("root_dir", logger::NoopLogger)
    }
}

pub type Scope = Vec<PathBuf>;

#[derive(Clone, Copy)]
pub enum Operation {
    Create,
    Update,
    Delete,
}

pub struct Engine<Log>(Context<Log>);

impl<Log> Engine<Log> {
    pub const fn new(context: Context<Log>) -> Self {
        Self(context)
    }

    pub const fn plan<'a>() -> Builder<Identity<(State<'a, Log>, ())>> {
        Builder::new()
    }

    pub fn execute<'a, Src, Res, Tx>(
        &'a self,
        plan: &Builder<Tx>,
        input: &Input<Src, Library>,
    ) -> (State<'a, Log>, Res)
    where
        Src: Source,
        Tx: Transform<In = (State<'a, Log>, ()), Out = (State<'a, Log>, Res)>,
    {
        let Input { source, .. } = input;
        let scope = source.resolve(&self.0.root_dir);

        let state = State::new(&self.0, scope);

        plan.execute((state, ()))
    }

    pub fn incremental<'a, Res, Tx, Ops>(
        prev: State<'a, Log>,
        plan: &Builder<Tx>,
        operations: Ops,
    ) -> (State<'a, Log>, Res)
    where
        Tx: Transform<In = (State<'a, Log>, ()), Out = (State<'a, Log>, Res)>,
        Ops: AsRef<[(PathBuf, Operation)]>,
    {
        let next = prev.evolve(operations);

        plan.execute((next, ()))
    }
}

#[cfg(test)]
mod tests {
    use super::{logger::NoopLogger, Context, Engine, Input};
    use kore::{internal, pipeline::Peek, str};
    use lang::ast;

    #[derive(Clone, Copy)]
    struct MockGenerator;

    impl internal::Generator for MockGenerator {
        type Input = ast::shape::Program;
        type Output = String;

        fn generate<T>(&self, path: T, _: Self::Input) -> (std::path::PathBuf, Self::Output)
        where
            T: AsRef<std::path::Path>,
        {
            (path.as_ref().with_extension("out"), str!("output"))
        }
    }

    #[test]
    fn one_off_execution() {
        let context = Context::mock();
        let engine = Engine::new(context);
        let input = Input::from_entry("entry", []);
        let pipeline = Engine::plan();

        engine.execute(&pipeline, &input);
    }

    #[test]
    #[ignore = "skip temporarily"]
    fn incremental_execution() {
        let context = Context::mock();
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
        Engine::<NoopLogger>::plan()
            .parse()
            .peek(|_| ())
            .link()
            .peek(|_| ())
            .analyze()
            .peek(|_| ())
            .generate(MockGenerator)
            .peek(|_| ())
            .write("out_dir");
    }

    #[test]
    fn format_pipeline() {
        Engine::<NoopLogger>::plan()
            .parse()
            .peek(|_| ())
            .format()
            .peek(|_| ())
            .write("out_dir");
    }

    #[test]
    fn flexible_pipeline() {
        Engine::<NoopLogger>::plan()
            .parse()
            .link()
            .analyze()
            .link()
            .analyze()
            .link()
            .generate(MockGenerator)
            .write("out_dir");
    }
}
