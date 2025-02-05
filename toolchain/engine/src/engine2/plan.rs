use super::{
    pipeline::{Container, Execute, Identity, Map, Peek, Transform},
    State,
};

pub struct Builder<T>(T);

impl Builder<Identity<(State, Vec<String>)>> {
    pub const fn new() -> Self {
        Self(Identity::new())
    }
}

impl<T> Container for Builder<T> {
    type Inner = T;

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

impl<T, F> Peek<T, F> for Builder<T>
where
    T: Transform,
    F: Fn(&T::Out),
{
}

impl<T> Execute<T> for Builder<T>
where
    T: Transform,
{
    fn execute(&self, input: T::In) -> T::Out {
        self.0.apply(input)
    }
}

impl<T> Builder<T>
where
    T: Transform<Out = (State, Vec<String>)>,
{
    pub fn parse(self) -> Builder<Parse<T>> {
        self.map(Parse)
    }
}

impl<T> Builder<T>
where
    T: Transform<Out = (State, Parsed)>,
{
    pub fn format(self) -> Builder<Format<T>> {
        self.map(Format)
    }
}

impl<T> Builder<T>
where
    T: Transform<Out = (State, Parsed)>,
{
    pub fn link(self) -> Builder<Link<T>> {
        self.map(Link)
    }
}

impl<T> Builder<T>
where
    T: Transform<Out = (State, Linked)>,
{
    pub fn analyze(self) -> Builder<Analyze<T>> {
        self.map(Analyze)
    }
}

impl<T> Builder<T>
where
    T: Transform<Out = (State, Analyzed)>,
{
    pub fn generate<G>(self, generator: G) -> Builder<Generate<T, G>> {
        self.map(|prev| Generate(prev, generator))
    }
}

/* parse */

pub struct Parsed;

pub struct Parse<T>(T);

impl<T> Transform for Parse<T>
where
    T: Transform<Out = (State, Vec<String>)>,
{
    type In = T::In;
    type Out = (State, Parsed);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);
        (state, Parsed)
    }
}

/* format */

pub struct Formatted;

pub struct Format<T>(T);

impl<T> Transform for Format<T>
where
    T: Transform<Out = (State, Parsed)>,
{
    type In = T::In;
    type Out = (State, Formatted);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);
        (state, Formatted)
    }
}

/* link */

pub struct Linked;

pub struct Link<T>(T);

impl<T> Transform for Link<T>
where
    T: Transform<Out = (State, Parsed)>,
{
    type In = T::In;
    type Out = (State, Linked);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);
        (state, Linked)
    }
}

/* analyze */

pub struct Analyzed;

pub struct Analyze<T>(T);

impl<T> Transform for Analyze<T>
where
    T: Transform<Out = (State, Linked)>,
{
    type In = T::In;
    type Out = (State, Analyzed);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);
        (state, Analyzed)
    }
}

/* generate */

pub struct Generated;

pub struct Generate<T, G>(T, G);

impl<T, G> Transform for Generate<T, G>
where
    T: Transform<Out = (State, Analyzed)>,
{
    type In = T::In;
    type Out = (State, Generated);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, _) = self.0.apply(input);
        (state, Generated)
    }
}
