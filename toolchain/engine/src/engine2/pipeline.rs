use std::marker::PhantomData;

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

pub struct Pipeline<T>(T);

impl<T> Pipeline<Identity<T>> {
    pub const fn new() -> Self {
        Self(Identity::new())
    }
}

impl<T> Pipeline<T> {
    pub fn chain<U, F>(self, f: F) -> Pipeline<U>
    where
        F: FnOnce(T) -> U,
    {
        Pipeline(f(self.0))
    }
}

impl<T> Pipeline<T>
where
    T: Transform,
{
    pub fn peek<F>(self, f: F) -> Pipeline<Peek<T, F>>
    where
        F: FnMut(&T::Out),
    {
        self.chain(|prev| Peek(prev, f))
    }
}

pub struct Peek<P, F>(P, F);

impl<P, F> Transform for Peek<P, F>
where
    P: Transform,
    F: FnMut(&P::Out),
{
    type In = P::In;
    type Out = P::Out;
}
