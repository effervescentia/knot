mod input;
mod link;
mod logger;
mod plan;
mod report;
mod resolve;
mod state;
mod validate;

pub use input::Input;
use kore::pipeline::{Execute, Identity, Transform};
pub use link::Link;
#[cfg(test)]
pub use logger::MemoryLogger;
pub use logger::{Logger, NoopLogger};
pub use plan::{Analyzed, Builder, Linked, Parsed};
pub use report::{
    CodeFrame, ConfigurationError, EnvironmentError, ExecutionError, Report, Reporter,
};
pub use resolve::{FileCache, FileSystem, MemoryCache, Resolver};
pub use state::State;
use std::path::{Path, PathBuf};

pub type Result<T> = std::result::Result<T, Box<Report>>;

pub type Scope = Vec<PathBuf>;

#[derive(Debug)]
pub struct Context<Log> {
    pub logger: Log,

    root_dir: PathBuf,
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
impl Context<NoopLogger> {
    pub fn mock() -> Self {
        Self::new("root_dir", NoopLogger)
    }
}

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

    pub const fn plan<'a>() -> Builder<Identity<State<'a, Log>, ()>> {
        Builder::new()
    }

    pub fn execute<'a, Src, Lib, Res, Tx>(
        &'a self,
        plan: &Builder<Tx>,
        input: &Input<Src, Lib>,
    ) -> (State<'a, Log>, Res)
    where
        Src: input::Source,
        Tx: Transform<Context = State<'a, Log>, In = (), Out = Res>,
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
        Tx: Transform<Context = State<'a, Log>, In = (), Out = Res>,
        Ops: AsRef<[(PathBuf, Operation)]>,
    {
        let next = prev.evolve(operations);

        plan.execute((next, ()))
    }
}
