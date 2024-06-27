#include <iostream>

using namespace std;

struct TreeNode{
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr){}
};

//查找二叉树最大深度
int maxDepth(TreeNode* root) {

    if (root == nullptr){
        return 0;
    }
    else {
        int leftDepth = maxDepth(root->left);
        int rightDepth = maxDepth(root->right);
        return max(leftDepth, rightDepth) + 1;
    }

}

//前序遍历  根节点->左子树->右子树
void preorder(TreeNode* root){

    if (root == nullptr){
        return;
    }

    cout << root->val << " ";
    preorder(root->left);
    preorder(root->right);

}

//后序遍历  左子树->右子树->根节点
void postorder(TreeNode* root){

    if (root == nullptr){
        return;
    }

    postorder(root->left);
    postorder(root->right);
    cout << root->val << " ";

}

//中序遍历  左子树->根节点->右子树
void inorder(TreeNode* root){

    if (root == nullptr){
        return;
    }
    inorder(root->left);
    cout << root->val << " ";
    inorder(root->right);

}

int main() {

    TreeNode* root = new TreeNode(1);

    root->left = new TreeNode(2);

    root->right = new TreeNode(3);


    root->left->left = new TreeNode(4);

    root->left->right = new TreeNode(5);


    postorder(root);

    int depth = maxDepth(root);
    cout << "depth : " << depth << endl;


    return 0;
}
