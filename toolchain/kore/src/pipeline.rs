use std::marker::PhantomData;

pub trait Container: Sized {
    type Inner;

    fn consume(self) -> Self::Inner;

    fn wrap(inner: Self::Inner) -> Self;
}

pub trait Map<In, Out>: Container<Inner = In> {
    type Result: Container<Inner = Out>;

    fn map<F>(self, f: F) -> Self::Result
    where
        F: FnOnce(In) -> Out,
    {
        Self::Result::wrap(f(self.consume()))
    }
}

pub trait Peek<T, F>: Map<T, Sink<T, F>>
where
    T: Transform,
    F: Fn(&(T::Context, T::Out)),
{
    fn peek(self, f: F) -> Self::Result {
        self.map(|x| Sink(x, f))
    }
}

pub trait Execute<T>: Container<Inner = T>
where
    T: Transform,
{
    fn execute(&self, input: (T::Context, T::In)) -> (T::Context, T::Out);
}

pub trait Transform: Sized {
    type Context;
    type In;
    type Out;

    fn apply(&self, input: (Self::Context, Self::In)) -> (Self::Context, Self::Out);
}

#[derive(Default)]
pub struct Identity<Context, Value>(PhantomData<Context>, PhantomData<Value>);

impl<Context, Value> Identity<Context, Value> {
    pub const fn new() -> Self {
        Self(PhantomData, PhantomData)
    }
}

impl<Context, Value> Transform for Identity<Context, Value> {
    type Context = Context;
    type In = Value;
    type Out = Value;

    fn apply(&self, input: (Self::Context, Self::In)) -> (Self::Context, Self::Out) {
        input
    }
}

pub struct Sink<P, F>(P, F);

impl<P, F> Transform for Sink<P, F>
where
    P: Transform,
    F: Fn(&(P::Context, P::Out)),
{
    type Context = P::Context;
    type In = P::In;
    type Out = P::Out;

    fn apply(&self, input: (Self::Context, Self::In)) -> (Self::Context, Self::Out) {
        let result = self.0.apply(input);
        self.1(&result);
        result
    }
}
