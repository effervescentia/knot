use super::{
    pipeline::{Container, Execute, Identity, Map, Peek, Transform},
    Context,
};
use std::{
    marker::PhantomData,
    path::{Path, PathBuf},
};

pub struct Builder<T, L>(T, PhantomData<L>);

impl<L> Builder<Identity<Context>, L> {
    pub const fn new() -> Self {
        Self(Identity::new(), PhantomData)
    }
}

impl<T, L> Container for Builder<T, L> {
    type Inner = T;

    fn consume(self) -> Self::Inner {
        self.0
    }

    fn wrap(inner: Self::Inner) -> Self {
        Self(inner, PhantomData)
    }
}

impl<In, Out, L> Map<In, Out> for Builder<In, L> {
    type Result = Builder<Out, L>;
}

impl<T, F, L> Peek<T, F> for Builder<T, L>
where
    T: Transform,
    F: Fn(&T::Out),
{
}

impl<T, L> Execute<T> for Builder<T, L>
where
    T: Transform,
{
    fn execute(&self, input: T::In) -> T::Out {
        self.0.apply(input)
    }
}

impl<T, L> Builder<T, L>
where
    T: Transform<Out = Context>,
{
    pub fn from_entry<E>(self, entry: E) -> Builder<FromEntry<T, E, L>, L>
    where
        E: AsRef<Path>,
    {
        self.map(|prev| FromEntry::new(prev, entry))
    }

    pub fn from_glob<G>(self, glob: G) -> Builder<FromGlob<T, G, L>, L>
    where
        G: AsRef<str>,
    {
        self.map(|prev| FromGlob::new(prev, glob))
    }
}

impl<T, S, L> Builder<T, L>
where
    T: Transform<Out = Input<S, L>>,
{
    pub fn with_libraries<V>(self, libraries: V) -> Builder<WithLibraries<T, L>, L>
    where
        V: AsRef<[L]>,
        L: Clone,
    {
        self.map(|prev| WithLibraries(prev, libraries.as_ref().to_vec()))
    }
}

pub struct Input<S, L> {
    pub source: S,
    pub libraries: Vec<L>,
}

impl<S, L> Input<S, L> {
    pub fn from_source(source: S) -> Self {
        Self {
            source,
            libraries: vec![],
        }
    }
}

pub trait Source {}

pub struct Entrypoint(PathBuf);

impl Source for Entrypoint {}

pub struct Glob(String);

impl Source for Glob {}

pub struct FromEntry<T, E, L>(T, E, PhantomData<L>);

impl<T, E, L> FromEntry<T, E, L> {
    pub const fn new(prev: T, entry: E) -> Self {
        Self(prev, entry, PhantomData)
    }
}

impl<T, E, L> Transform for FromEntry<T, E, L>
where
    T: Transform<Out = Context>,
    E: AsRef<Path>,
{
    type In = T::In;
    type Out = Input<Entrypoint, L>;

    fn apply(&self, input: Self::In) -> Self::Out {
        let context = self.0.apply(input);
        Input::from_source(Entrypoint(self.1.as_ref().to_path_buf()))
    }
}

pub struct FromGlob<T, G, L>(T, G, PhantomData<L>);

impl<T, G, L> FromGlob<T, G, L> {
    pub const fn new(prev: T, glob: G) -> Self {
        Self(prev, glob, PhantomData)
    }
}

impl<T, G, L> Transform for FromGlob<T, G, L>
where
    T: Transform<Out = Context>,
    G: AsRef<str>,
{
    type In = T::In;
    type Out = Input<Glob, L>;

    fn apply(&self, input: Self::In) -> Self::Out {
        let context = self.0.apply(input);
        Input::from_source(Glob(self.1.as_ref().to_owned()))
    }
}

pub struct WithLibraries<T, L>(T, Vec<L>);

impl<T, L, S> Transform for WithLibraries<T, L>
where
    T: Transform<Out = Input<S, L>>,
    L: Clone,
{
    type In = T::In;
    type Out = T::Out;

    fn apply(&self, input: Self::In) -> Self::Out {
        let mut result = self.0.apply(input);
        result.libraries.extend(self.1.iter().cloned());
        result
    }
}
