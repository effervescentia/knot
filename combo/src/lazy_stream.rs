use std::cell::OnceCell;

// use crate::Lazy3;

// pub enum List<'a, T> {
//     Cons((T, Box<Lazy3<'a, List<'a, T>>>)),
//     Nil,
// }

// impl<'a, T> List<'a, T> {}

// impl<'a, T> List<'a, T> {
//     pub fn from_vec(mut vec: Vec<T>) {
//         fn next<'b, U>(iter: &'b mut impl Iterator<Item = U>) -> List<'b, U> {}

//         // next(vec.iter())
//         // fn next<'b, U>(iter: &'b mut impl Iterator<Item = U>) -> List<'b, U> {
//         //     match iter.next() {
//         //         // Some(value) => List::Cons((value, Lazy::new(Box::new(|| Box::new(next(iter)))))),
//         //         Some(mut value) => {
//         //             let mut factory = || value;
//         //             let lazy = Lazy2::new(&mut factory);
//         //             // let mut factory = || next(iter);
//         //             // List::Cons((value, Box::new(Lazy2::new(&mut factory))))

//         //             List::Nil
//         //         }
//         //         None => List::Nil,
//         //     }
//         // }
//         // fn next<'b, U>(mut iter: impl Iterator<Item = U>) -> List<'b, U> {
//         //     match iter.next() {
//         //         // Some(value) => List::Cons((value, Lazy::new(Box::new(|| Box::new(next(iter)))))),
//         //         Some(value) => {
//         //             let mut factory = || next(iter);
//         //             List::Cons((value, Box::new(Lazy2::new(&mut factory))))
//         //         }
//         //         None => List::Nil,
//         //     }
//         // }
//     }
// }

// use std::slice::Iter;

// pub struct Stream<T, I: Iterator<Item = T>> {
//     iter: I,
// }

// pub enum Stream<T> {
//     Cons((T, Lazy<Box<Stream<T>>, Box<dyn FnMut() -> Box<Stream<T>>>>)),
//     Nil,
// }

// impl<T> Stream<T> {
//     // fn next(&mut self) -> Stream<T> {
//     //     match self.iter.next() {
//     //         Some(value) => Str::Cons((value, || Box::new(Stream::next(self)))),
//     //         None => Str::Nil,
//     //     }
//     // }

//     fn from_vec<U>(vec: Vec<U>) -> Stream<U> {
//         fn next<V>(mut iter: slice::Iter<V>) -> Stream<V> {
//             match iter.next() {
//                 Some(value) => Stream::Cons((*value, Lazy::new(Box::new(|| Box::new(next(iter)))))),
//                 None => Stream::Nil,
//             }
//         }

//         next(vec.iter())
//     }
// }

// fn do_thing() {
//     let vec = vec![1, 2, 3];
//     let lazy_iter = vec.iter();

//     fn next<'a, I: Iterator<Item = &'a i32>>(mut iter: I) {
//         match iter.next() {
//             Some(value) => Some((value, Lazy::new(|| next(iter)))),
//             None => None,
//         };
//     }

//     let lazy = next(lazy_iter);
//     // lazy_iter.map(|item| Lazy::new(next));
// }

// struct LazyIterator<'a, T, F: FnMut() -> LazyIterator<'a, T, F>>(
//     Option<(T, Lazy<LazyIterator<'a, T, F>, F>)>,
// );

// struct LazyStream<'a, T> {
//     iter: slice::Iter<'a, T>,
// }

// impl<'a, T> LazyStream<'a, T> {
//     pub fn from_iter(iter: slice::Iter<'a, T>) -> Option<(T,)> {
//         LazyStream { iter }
//     }

//     // pub fn from_vec(vec: &'a Vec<T>) -> LazyStream<'a, T> {
//     //     LazyStream::from_iter(vec.iter())
//     // }

//     // pub fn from_iter(iter: slice::Iter<'a, T>) -> LazyStream<'a, T> {

//     //     LazyStream { iter }
//     // }

//     // pub fn from_vec(vec: &'a Vec<T>) -> LazyStream<'a, T> {
//     //     LazyStream::from_iter(vec.iter())
//     // }
// }

// #[cfg(test)]
// mod tests {
//     use super::*;

//     #[test]
//     fn new_from_vec() {
//         let stream = LazyStream::from_vec(&vec![1, 2, 3]);
//     }
// }
