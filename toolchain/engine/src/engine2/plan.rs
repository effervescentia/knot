use super::{
    pipeline::{Chain, Container, Identity, Map, Peek, Transform},
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
    F: FnMut(&T::Out),
{
}

impl<T, U, F> Chain<T, U, F> for Builder<T>
where
    T: Transform,
    F: FnOnce(T) -> U,
{
}

impl<T> Builder<T>
where
    T: Transform<Out = (State, Vec<String>)>,
{
    pub fn parse(self) -> Builder<Parse<T>> {
        self.chain(Parse)
    }
}

impl<T> Builder<T>
where
    T: Transform<Out = (State, Parsed)>,
{
    pub fn link(self) -> Builder<Link<T>> {
        self.chain(Link)
    }
}

impl<T> Builder<T>
where
    T: Transform<Out = (State, Linked)>,
{
    pub fn analyze(self) -> Builder<Analyze<T>> {
        self.chain(Analyze)
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
}
