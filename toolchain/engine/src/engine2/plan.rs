use super::{
    pipeline::{Chain, Identity, Map, Peek, Pipeline, Transform},
    State,
};

pub struct Builder<T>(Pipeline<T>);

impl Builder<Identity<(State, Vec<String>)>> {
    pub const fn new() -> Self {
        Self(Pipeline::new())
    }
}

impl<T> From<Pipeline<T>> for Builder<T> {
    fn from(value: Pipeline<T>) -> Self {
        Self(value)
    }
}

impl<T> From<Builder<T>> for Pipeline<T> {
    fn from(value: Builder<T>) -> Self {
        value.0
    }
}

impl<In, Out> Map<Pipeline<In>, Pipeline<Out>> for Builder<In> {
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
