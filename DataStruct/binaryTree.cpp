#include <iostream>
#include <stack>
#include <vector>
#include <queue>
using namespace std;

/*
        5
       / \
      4   6
     / \   \
    1   2   0
*/

struct TreeNode{
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr){}
};

void preorder_loop(TreeNode* root){

    if (root == nullptr){
        return;
    }

    stack<TreeNode*> st;
    st.push(root);
    while (st.empty()){

        TreeNode* node = st.top();
        st.pop();

        cout << node->val << " ";
        if (node->right != nullptr){
            st.push(node->right);
        }
        if (node->left != nullptr){
            st.push(node->left);
        }
    }

}


vector<vector<int>> levelOrder_loop(TreeNode* root){

    queue<TreeNode*> que;
    if (root == nullptr){
        return {};
    }
    que.push(root);
    vector<vector<int>> result;
    while (!que.empty()){

        int size = que.size();
        vector<int> vec;
        for (int i = 0; i < size; ++i) {

            TreeNode* node = que.front();
            que.pop();
            vec.push_back(node->val);
            if(node->left) que.push(node->left);
            if(node->right) que.push(node->right);

        }
        result.push_back(vec);
    }
    return result;
}



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
