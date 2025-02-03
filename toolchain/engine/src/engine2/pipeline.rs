use std::marker::PhantomData;

pub trait Map<In, Out>: Into<In> {
    type Result: From<Out>;

    fn map<F>(self, f: F) -> Self::Result
    where
        F: FnOnce(In) -> Out,
    {
        Self::Result::from(f(self.into()))
    }
}

pub trait Peek<T, F>: Map<Pipeline<T>, Pipeline<Sink<T, F>>>
where
    T: Transform,
    F: FnMut(&T::Out),
{
    fn peek(self, f: F) -> Self::Result
    where
        F: FnMut(&T::Out),
    {
        self.map(|x| x.peek(f))
    }
}

pub trait Chain<T, U, F>: Map<Pipeline<T>, Pipeline<U>>
where
    T: Transform,
    F: FnOnce(T) -> U,
{
    fn chain(self, f: F) -> Self::Result {
        self.map(|x| x.chain(f))
    }
}

pub trait Transform: Sized {
    type In;
    type Out;

    // fn apply(&mut self, input: Self::In) -> Self::Out;
}

pub struct Identity<T>(PhantomData<T>);

impl<T> Identity<T> {
    pub const fn new() -> Self {
        Self(PhantomData)
    }
}

impl<T> Transform for Identity<T> {
    type In = T;
    type Out = T;
}

pub struct Sink<P, F>(P, F);

impl<P, F> Transform for Sink<P, F>
where
    P: Transform,
    F: FnMut(&P::Out),
{
    type In = P::In;
    type Out = P::Out;
}

pub struct Pipeline<T>(T);

impl<T> Pipeline<Identity<T>> {
    pub const fn new() -> Self {
        Self(Identity::new())
    }
}

impl<T> Pipeline<T>
where
    T: Transform,
{
    pub fn chain<F, U>(self, f: F) -> Pipeline<U>
    where
        F: FnOnce(T) -> U,
    {
        Pipeline(f(self.0))
    }

    pub fn peek<F>(self, f: F) -> Pipeline<Sink<T, F>>
    where
        F: FnMut(&T::Out),
    {
        self.chain(|prev| Sink(prev, f))
    }
}
