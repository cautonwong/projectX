macro_rules! my_macro {
    () => {
        println!("This is my macro!");
    };
}

macro_rules! json_macro {
    () => {
        println!("This is another macro!");
    };
}

fn main() {
    println!("Hello, world!");
    my_macro!();
    json_macro!();
}
