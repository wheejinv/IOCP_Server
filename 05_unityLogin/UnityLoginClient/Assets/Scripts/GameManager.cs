using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

using Assets.Class;

public class GameManager : MonoBehaviour {

    [Header("LoginPanel")]
    public InputField InputField_ID;
    public InputField InputField_Password;

    // Use this for initialization
    void Start () {
        NetworkSocket.Instance.SetupSocket();

        NetworkSocket.Instance.SendSocket("test msg");

    }

    void Update()
    {
        string recvString = NetworkSocket.Instance.RecvSocket();

        if( recvString != "" )
        {
            Debug.Log("recv from server : " + recvString);
        }
    }

    public void LoginBtn()
    {
        StartCoroutine(LoginCo());
    }

    IEnumerator LoginCo()
    {
        Debug.Log(InputField_ID.text);
        Debug.Log(InputField_Password.text);

        yield return null;
    }
}
