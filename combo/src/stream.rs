use std::cell::RefCell;

use crate::Lazy;

pub enum Stream<'a, T> {
    Next(T, Box<dyn FnOnce() -> Stream<'a, T> + 'a>),
    Nil,
}

impl<'a, T: 'a> Stream<'a, T> {
    fn from_iter_internal<I>(iter: RefCell<I>) -> Stream<'a, T>
    where
        I: Iterator<Item = T> + 'a,
    {
        let next = iter.borrow_mut().next();
        match next {
            Some(value) => Stream::Next(value, Box::new(|| Self::from_iter_internal(iter))),

            None => Stream::Nil,
        }
    }

    pub fn from_vec(vec: &'a [T]) -> Stream<'a, &T> {
        let cell = RefCell::new(vec.into_iter());
        Stream::from_iter_internal(cell)
    }

    pub fn from_string(string: &'a str) -> Stream<'a, char> {
        let cell = RefCell::new(string.chars());
        Stream::from_iter_internal(cell)
    }
}

pub enum LazyStream<'a, T> {
    Next(T, Box<Lazy<'a, LazyStream<'a, T>>>),
    Nil,
}

impl<'a, T: 'a> LazyStream<'a, T> {
    fn from_stream(stream: Stream<'a, T>) -> LazyStream<'a, T> {
        match stream {
            Stream::Next(value, next) => {
                let mut advance = || LazyStream::from_stream(next());
                LazyStream::Next(value, Box::new(Lazy::new(&mut advance)))
            }
            Stream::Nil => LazyStream::Nil,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn create_from_vec() {
        let vec = vec![1, 2, 3];
        let stream = Stream::from_vec(&vec);

        match stream {
            Stream::Next(first, next) => {
                assert_eq!(*first, 1);

                match next() {
                    Stream::Next(second, _) => {
                        assert_eq!(*second, 2)
                    }

                    Stream::Nil => panic!("Stream should not be empty"),
                }
            }
            Stream::Nil => panic!("Stream should not be empty"),
        }
    }
}
