mod pipeline;

use pipeline::{Pipeline, Transform};
use std::{marker::PhantomData, path::Path};

/*
phases

1. empty
2. collect inputs (entry, globs, libraries)
3. parse (optional discovery)
4. link
5. analyze
6. generate
*/

pub struct Context;

pub struct State;

pub struct Engine(Context);

impl Engine {
    pub fn new(context: Context) -> Self {
        Self(context)
    }

    pub fn pipeline() -> pipeline::Pipeline<pipeline::Identity<(State, Context)>> {
        pipeline::Pipeline::new()
    }

    pub fn execute<R, T>(&self, pipeline: &Pipeline<T>) -> (State, R)
    where
        T: Transform<In = (State, Context), Out = (State, R)>,
    {
        (State, todo!())
    }

    pub fn incremental<R, T>(
        &self,
        pipeline: &Pipeline<T>,
        state: State,
        targets: Vec<&str>,
    ) -> (State, R)
    where
        T: Transform<In = (State, Context), Out = (State, R)>,
    {
        (state, todo!())
    }
}

pub struct Inputs<S> {
    libraries: Vec<()>,
    source: S,
}

impl<S> Inputs<S> {
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

impl<T> Pipeline<T>
where
    T: Transform,
{
    pub fn from_entry<E>(self, entry: E) -> Pipeline<FromEntry<T, E>>
    where
        E: AsRef<Path>,
    {
        self.chain(|prev| FromEntry(prev, entry))
    }

    pub fn from_glob<G>(self, glob: G) -> Pipeline<FromGlob<T, G>>
    where
        G: AsRef<str>,
    {
        self.chain(|prev| FromGlob(prev, glob))
    }
}

pub struct FromEntry<T, E>(T, E);

impl<T, E> Transform for FromEntry<T, E>
where
    T: Transform<Out = (State, Context)>,
    E: AsRef<Path>,
{
    type In = T::In;
    type Out = (State, Inputs<Entrypoint<E>>);
}

pub struct FromGlob<T, G>(T, G);

impl<T, G> Transform for FromGlob<T, G>
where
    T: Transform<Out = (State, Context)>,
    G: AsRef<str>,
{
    type In = T::In;
    type Out = (State, Inputs<Glob<G>>);
}

impl<T, S> Pipeline<T>
where
    T: Transform<Out = (State, Inputs<S>)>,
{
    pub fn with_libraries<V, L>(self, libraries: V) -> pipeline::Pipeline<WithLibraries<T, L>>
    where
        V: AsRef<[L]>,
        L: Clone,
    {
        self.chain(|prev| WithLibraries(prev, libraries.as_ref().to_vec()))
    }

    pub fn parse(self) -> pipeline::Pipeline<Parse<T>> {
        self.chain(Parse)
    }
}

pub struct WithLibraries<T, L>(T, Vec<L>);

impl<T, L, S> Transform for WithLibraries<T, L>
where
    T: Transform<Out = (State, Inputs<S>)>,
{
    type In = T::In;
    type Out = T::Out;
}

pub struct Parsed<S> {
    inputs: Inputs<S>,
}

pub struct Parse<T>(T);

impl<T, S> Transform for Parse<T>
where
    T: Transform<Out = (State, Inputs<S>)>,
{
    type In = T::In;
    type Out = (State, Parsed<S>);
}

impl<T, S> Pipeline<T>
where
    T: Transform<Out = (State, Parsed<S>)>,
{
    pub fn link(self) -> pipeline::Pipeline<Link<T>> {
        self.chain(Link)
    }
}

pub struct Linked<S> {
    inputs: Inputs<S>,
}

pub struct Link<T>(T);

impl<T, S> Transform for Link<T>
where
    T: Transform<Out = (State, Parsed<S>)>,
{
    type In = T::In;
    type Out = (State, Linked<S>);
}

impl<T, S> Pipeline<T>
where
    T: Transform<Out = (State, Linked<S>)>,
{
    pub fn analyze(self) -> pipeline::Pipeline<Analyze<T>> {
        self.chain(Analyze)
    }
}

pub struct Analyzed<S> {
    inputs: Inputs<S>,
}

pub struct Analyze<T>(T);

impl<T, S> Transform for Analyze<T>
where
    T: Transform<Out = (State, Linked<S>)>,
{
    type In = T::In;
    type Out = (State, Analyzed<S>);
}

#[cfg(test)]
mod tests {
    use super::{pipeline::Transform, Context, Engine, FromEntry, Inputs, State};

    #[test]
    fn one_off_execution() {
        let context = Context;
        let engine = Engine::new(context);
        let pipeline = Engine::pipeline();

        let (_, result) = engine.execute(&pipeline);
    }

    #[test]
    fn incremental_execution() {
        let context = Context;
        let engine = Engine::new(context);
        let pipeline = Engine::pipeline();

        let (mut state, _) = engine.execute(&pipeline);

        loop {
            (state, _) = engine.incremental(&pipeline, state, vec![]);
        }
    }

    #[test]
    fn standard_build_pipeline() {
        // let context = Context;
        // let engine = Engine::new(context);
        let pipeline = Engine::pipeline()
            .from_entry("")
            .peek(|_| ())
            .with_libraries([""])
            .peek(|_| ())
            .parse()
            .peek(|_| ())
            .link()
            .peek(|_| ())
            .analyze()
            .peek(|_| ());

        // .chain(WithLibraries(vec![]))
        // .chain(Peek::new(|_: (State, Inputs<Engine>)| ()));

        // let (_, result) = engine.execute(&pipeline);
    }
}
