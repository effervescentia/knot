use std::{cell::RefCell, rc::Rc};

enum Stream<'a, T> {
    Next(Rc<T>, Box<dyn FnOnce() -> Stream<'a, T> + 'a>),
    Nil,
}

fn iter_to_stream<'a, T, I>(iter: &'a RefCell<I>) -> Stream<'a, T>
where
    I: Iterator<Item = T>,
{
    match iter.borrow_mut().next() {
        Some(value) => {
            let value_rc = Rc::new(value);
            let next = || iter_to_stream(iter);

            Stream::Next(value_rc, Box::new(next))
        }
        None => Stream::Nil,
    }
}

fn vec_to_stream() {
    let vec = vec![1, 2, 3];
    let cell = RefCell::new(vec.into_iter());

    iter_to_stream(&cell);
}
