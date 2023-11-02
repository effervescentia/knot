use crate::Identity;
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub struct Node<T, R, C>(pub T, pub R, pub C);

impl<T, R, C> Node<T, R, C> {
    pub const fn value(&self) -> &T {
        &self.0
    }

    pub const fn range(&self) -> &R {
        &self.1
    }

    pub const fn context(&self) -> &C {
        &self.2
    }

    pub fn with_context<C2>(self, context: C2) -> Node<T, R, C2> {
        Node(self.0, self.1, context)
    }

    pub fn map_value<T2>(self, f: impl FnOnce(T) -> T2) -> Node<T2, R, C> {
        Node(f(self.0), self.1, self.2)
    }

    pub fn map_range(self, f: impl FnOnce(R) -> R) -> Self {
        Self(self.0, f(self.1), self.2)
    }
}

impl<T, R> Node<T, R, ()> {
    pub const fn raw(x: T, range: R) -> Self {
        Self(x, range, ())
    }
}

impl<T, R, C> Node<T, R, C>
where
    C: Identity<usize>,
{
    pub fn id(&self) -> &usize {
        self.2.id()
    }
}

impl<T, R, C> Clone for Node<T, R, C>
where
    T: Clone,
    R: Copy,
    C: Clone,
{
    fn clone(&self) -> Self {
        Self(self.0.clone(), self.1, self.2.clone())
    }
}
