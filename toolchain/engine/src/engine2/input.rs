use super::{
    pipeline::{Chain, Container, Identity, Map, Peek, Transform},
    Context,
};
use std::path::Path;

pub struct Builder<T>(T);

impl Builder<Identity<Context>> {
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
    T: Transform<Out = Context>,
{
    pub fn from_entry<E>(self, entry: E) -> Builder<FromEntry<T, E>>
    where
        E: AsRef<Path>,
    {
        self.chain(|prev| FromEntry(prev, entry))
    }

    pub fn from_glob<G>(self, glob: G) -> Builder<FromGlob<T, G>>
    where
        G: AsRef<str>,
    {
        self.chain(|prev| FromGlob(prev, glob))
    }
}

impl<T, S> Builder<T>
where
    T: Transform<Out = Input<S>>,
{
    pub fn with_libraries<V, L>(self, libraries: V) -> Builder<WithLibraries<T, L>>
    where
        V: AsRef<[L]>,
        L: Clone,
    {
        self.chain(|prev| WithLibraries(prev, libraries.as_ref().to_vec()))
    }
}

pub struct Input<S> {
    libraries: Vec<()>,
    source: S,
}

impl<S> Input<S> {
    pub fn from_source(source: S) -> Self {
        Self {
            source,
            libraries: vec![],
        }
    }
}

pub trait Source {}

pub struct Entrypoint<T>(T);

impl<T> Source for Entrypoint<T> where T: AsRef<Path> {}

pub struct Glob<T>(T);

impl<T> Source for Glob<T> where T: AsRef<str> {}

pub struct FromEntry<T, E>(T, E);

impl<T, E> Transform for FromEntry<T, E>
where
    T: Transform<Out = Context>,
    E: AsRef<Path>,
{
    type In = T::In;
    type Out = Input<Entrypoint<E>>;
}

pub struct FromGlob<T, G>(T, G);

impl<T, G> Transform for FromGlob<T, G>
where
    T: Transform<Out = Context>,
    G: AsRef<str>,
{
    type In = T::In;
    type Out = Input<Glob<G>>;
}

pub struct WithLibraries<T, L>(T, Vec<L>);

impl<T, L, S> Transform for WithLibraries<T, L>
where
    T: Transform<Out = Input<S>>,
{
    type In = T::In;
    type Out = T::Out;
}
