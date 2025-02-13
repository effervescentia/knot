mod link;
mod parse;

use super::{
    pipeline::{Container, Execute, Identity, Map, Peek, Transform},
    State,
};
use link::{Link, Linked};
use parse::{Parse, Parsed};

pub struct Builder<Tx>(Tx);

impl Builder<Identity<(State<'_>, ())>> {
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

impl<'a, Tx> Builder<Tx>
where
    Tx: Transform<Out = (State<'a>, ())>,
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

impl<'a, Tx> Builder<Tx>
where
    Tx: Transform<Out = (State<'a>, Parsed)>,
{
    /** transform internal modules using the standard formatter */
    pub fn format(self) -> Builder<Format<Tx>> {
        self.map(Format)
    }
}

impl<'a, Tx> Builder<Tx>
where
    Tx: Transform<Out = (State<'a>, Parsed)>,
{
    /** record links between internal modules and their external dependencies (libraries) */
    pub fn link(self) -> Builder<Link<Tx>> {
        self.map(Link)
    }
}

impl<'a, Tx> Builder<Tx>
where
    Tx: Transform<Out = (State<'a>, Linked)>,
{
    /** check if the code is semantically correct and determine types of all values */
    pub fn analyze(self) -> Builder<Analyze<Tx>> {
        self.map(Analyze)
    }
}

impl<'a, Tx> Builder<Tx>
where
    Tx: Transform<Out = (State<'a>, Analyzed)>,
{
    pub fn generate<G>(self, generator: G) -> Builder<Generate<Tx, G>> {
        self.map(|prev| Generate(prev, generator))
    }
}

/* format */

pub struct Formatted;

pub struct Format<Tx>(Tx);

impl<'a, Tx> Transform for Format<Tx>
where
    Tx: Transform<Out = (State<'a>, Parsed)>,
{
    type In = Tx::In;
    type Out = (State<'a>, Formatted);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);
        (state, Formatted)
    }
}

/* analyze */

pub struct Analyzed;

pub struct Analyze<Tx>(Tx);

impl<'a, Tx> Transform for Analyze<Tx>
where
    Tx: Transform<Out = (State<'a>, Linked)>,
{
    type In = Tx::In;
    type Out = (State<'a>, Analyzed);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);
        (state, Analyzed)
    }
}

/* generate */

pub struct Generated;

pub struct Generate<Tx, Gen>(Tx, Gen);

impl<'a, Tx, G> Transform for Generate<Tx, G>
where
    Tx: Transform<Out = (State<'a>, Analyzed)>,
{
    type In = Tx::In;
    type Out = (State<'a>, Generated);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);
        (state, Generated)
    }
}
