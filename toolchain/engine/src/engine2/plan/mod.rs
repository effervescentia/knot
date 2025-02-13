mod analyze;
mod format;
mod generate;
mod link;
mod parse;
mod write;

use super::{
    pipeline::{Container, Execute, Identity, Map, Peek, Transform},
    State,
};
use analyze::Analyze;
pub use analyze::Analyzed;
use format::Format;
use generate::Generate;
use link::Link;
pub use link::Linked;
use parse::Parse;
pub use parse::Parsed;
use write::{Output, Write};

#[derive(Default)]
pub struct Builder<Tx>(Tx);

impl<Log> Builder<Identity<(State<'_, Log>, ())>> {
    pub const fn new() -> Self {
        Self(Identity::new())
    }
}

impl<Tx> Container for Builder<Tx> {
    type Inner = Tx;

    fn consume(self) -> Self::Inner {
        self.0
    }

    fn wrap(inner: Self::Inner) -> Self {
        Self(inner)
    }
}

impl<In, Out> Map<In, Out> for Builder<In> {
    type Result = Builder<Out>;
}

impl<Tx, F> Peek<Tx, F> for Builder<Tx>
where
    Tx: Transform,
    F: Fn(&Tx::Out),
{
}

impl<Tx> Execute<Tx> for Builder<Tx>
where
    Tx: Transform,
{
    fn execute(&self, input: Tx::In) -> Tx::Out {
        self.0.apply(input)
    }
}

impl<'a, Tx, Log> Builder<Tx>
where
    Tx: Transform<Out = (State<'a, Log>, ())>,
    Log: 'a,
{
    /** load and parse internal modules without following dependencies */
    pub fn parse(self) -> Builder<Parse<Tx>> {
        self.map(Parse::bind(parse::Options {
            follow_imports: false,
        }))
    }

    /** load and parse internal modules and follow dependencies */
    pub fn parse_and_traverse(self) -> Builder<Parse<Tx>> {
        self.map(Parse::bind(parse::Options {
            follow_imports: true,
        }))
    }
}

impl<'a, Tx, Log> Builder<Tx>
where
    Tx: Transform<Out = (State<'a, Log>, Parsed)>,
    Log: 'a,
{
    /** transform internal modules using the standard formatter */
    pub fn format(self) -> Builder<Format<Tx>> {
        self.map(Format::new)
    }
}

impl<'a, Tx, Log> Builder<Tx>
where
    Tx: Transform<Out = (State<'a, Log>, Parsed)>,
    Log: 'a,
{
    /** record links between internal modules and their external dependencies (libraries) */
    pub fn link(self) -> Builder<Link<Tx>> {
        self.map(Link::new)
    }
}

impl<'a, Tx, Log> Builder<Tx>
where
    Tx: Transform<Out = (State<'a, Log>, Linked)>,
    Log: 'a,
{
    /** check if the code is semantically correct and determine types of all values */
    pub fn analyze(self) -> Builder<Analyze<Tx>> {
        self.map(Analyze::new)
    }
}

impl<'a, Tx, Log> Builder<Tx>
where
    Tx: Transform<Out = (State<'a, Log>, Analyzed)>,
    Log: 'a,
{
    pub fn generate<G>(self, generator: G) -> Builder<Generate<Tx, G>> {
        self.map(Generate::bind(generator))
    }
}

impl<'a, Tx, Res, Log> Builder<Tx>
where
    Tx: Transform<Out = (State<'a, Log>, Res)>,
    Res: Output,
    Log: 'a,
{
    pub fn write(self) -> Builder<Write<Tx>> {
        self.map(Write::new)
    }
}
