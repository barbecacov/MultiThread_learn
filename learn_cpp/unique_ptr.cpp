#include <iostream>
#include <memory>
#include <string>

// 模拟结果包装类（类似Result<T>）
template <typename T>
class Result {
private:
    bool success_;
    T value_;          // 成功时存储的值
    std::string error_; // 失败时的错误信息

public:
    // 成功构造函数
    Result(T value) : success_(true), value_(std::move(value)) {}

    // 失败构造函数
    Result(std::string error) : success_(false), error_(std::move(error)) {}

    // 重载*运算符，获取内部值（仅当成功时）
    T& operator*() {
        if (!success_) {
            throw std::runtime_error("访问失败的Result值");
        }
        return value_;
    }

    bool is_success() const { return success_; }
    const std::string& error() const { return error_; }
};

// 学生类
class Student {
private:
    std::string name_;
    int age_;

public:
    // 构造函数
    Student(std::string name, int age) : name_(std::move(name)), age_(age) {
        std::cout << "Student构造: " << name_ << std::endl;
    }

    // 移动构造函数
    Student(Student&& other) noexcept
        : name_(std::move(other.name_)), age_(other.age_) {
        std::cout << "Student移动构造: " << name_ << std::endl;
    }

    // 禁止复制（模拟资源独占场景）
    Student(const Student&) = delete;
    Student& operator=(const Student&) = delete;

    void print() const {
        std::cout << "姓名: " << name_ << ", 年龄: " << age_ << std::endl;
    }

    ~Student() {
        std::cout << "Student析构: " << name_ << std::endl;
    }
};

// 模拟从某处加载Student对象（可能失败）
Result<Student> load_student(const std::string& name, int age) {
    // 这里简化处理，直接返回成功结果
    return Result<Student>(Student(name, age));
}

int main() {
    // 1. 加载Student对象，结果用Result<Student>包装
    Result<Student> student_result = load_student("张三", 18);

    if (!student_result.is_success()) {
        std::cout << "加载失败: " << student_result.error() << std::endl;
        return -1;
    }

    // 2. 定义unique_ptr智能指针，用于管理Student对象
    std::unique_ptr<Student> student_ptr;

    // 3. 将Result中的Student对象转移给unique_ptr管理
    // *student_result: 获取Result内部的Student对象
    // std::move: 转移对象所有权（触发移动构造）
    student_ptr = std::make_unique<Student>(std::move(*student_result));

    // 使用智能指针管理的对象
    student_ptr->print();

    // 离开作用域时，unique_ptr会自动释放Student对象
    return 0;
}
