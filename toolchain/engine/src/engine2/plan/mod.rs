mod parse;

use super::{
    pipeline::{Container, Execute, Identity, Map, Peek, Transform},
    Scope, State,
};
use parse::Parsed;

pub struct Builder<Tx>(Tx);

impl<'a> Builder<Identity<(State<'a>, ())>> {
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
    pub fn parse(self) -> Builder<parse::Parse<Tx>> {
        self.map(parse::Parse)
    }
}

impl<'a, Tx> Builder<Tx>
where
    Tx: Transform<Out = (State<'a>, Parsed)>,
{
    pub fn format(self) -> Builder<Format<Tx>> {
        self.map(Format)
    }
}

impl<'a, Tx> Builder<Tx>
where
    Tx: Transform<Out = (State<'a>, Parsed)>,
{
    pub fn link(self) -> Builder<Link<Tx>> {
        self.map(Link)
    }
}

impl<'a, Tx> Builder<Tx>
where
    Tx: Transform<Out = (State<'a>, Linked)>,
{
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

/* link */

pub struct Linked;

pub struct Link<Tx>(Tx);

impl<'a, Tx> Transform for Link<Tx>
where
    Tx: Transform<Out = (State<'a>, Parsed)>,
{
    type In = Tx::In;
    type Out = (State<'a>, Linked);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);
        (state, Linked)
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
